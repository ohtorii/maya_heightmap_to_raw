#include <windows.h>
#include <imagehlp.h>

#include <maya/MFnPlugin.h>
#include <maya/MStatus.h>	  
#include <maya/MEvent.h> 
#include <maya/MGlobal.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MDagPath.h>
#include <maya/MFn.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNumericAttribute.h>

#include <maya/MString.h>
#include <maya/MArgList.h>
//#include <maya/MFnPlugin.h>
#include <maya/MSyntax.h>
#include <maya/MPxCommand.h>
#include <maya/MObject.h>
#include <maya/MArgDatabase.h>

#include <maya/MDistance.h>
#include <maya/MItDag.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MIOStream.h>
//#include <maya/MSimple.h>

#include<vector>
#include<opencv2/opencv.hpp>

#pragma comment(lib, "imagehlp.lib")


#define COMMAND_NAME			"HeigtMapToRaw"

#define FLAG_WIDTH				"-w"
#define FLAG_LONG_WIDTH			"-width"
#define FLAG_HEIGHT				"-h"
#define FLAG_LONG_HEIGHT		"-height"
#define FLAG_OUTPUT_DIR			"-o"
#define FLAG_LONG_OUTPUT_DIR	"-output_dir"



class PolyMeshInfo {
public:
	PolyMeshInfo() {
		m_width = 0.0;
		m_height = 0.0;
		m_subdivisionsWidth = 0;
		m_subdivisionsHeight = 0;
	};
	double m_width, m_height;
	int m_subdivisionsWidth, m_subdivisionsHeight;
};


class Heightmap {
public:
	Heightmap() {

	};
	//・ｽ鞫懶ｿｽﾌ包ｿｽ・ｽ・ｽ・ｽ謫ｾ・ｽ・ｽ・ｽ・ｽ
	int GetImageWidth()const {
		return m_poly_mesh.m_subdivisionsWidth + 1;
	};
	//・ｽ鞫懶ｿｽﾌ搾ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ謫ｾ・ｽ・ｽ・ｽ・ｽ
	int GetImageHeight()const {
		return m_poly_mesh.m_subdivisionsHeight + 1;
	}

	std::vector<unsigned short>	m_original_image;
	//std::vector<unsigned short>	m_resized_image;
	std::string		m_node_name;
	PolyMeshInfo	m_poly_mesh;
};


class CommnadArgument {
public:
	CommnadArgument() {
		m_width = 0;
		m_height = 0;
	};
	int			m_width;
	int			m_height;
	MString		m_output_dir;
};

//DeclareSimpleCommand(heightmap_to_raw, PLUGIN_COMPANY, "4.5");
class HeigtmapToRaw : public MPxCommand
{
public:
	// Constructor / Destructor methods
	//
	HeigtmapToRaw();
	~HeigtmapToRaw() override;

	// Creator method
	//
	static void*	creator();

	// Syntax methods
	//
	//virtual bool		hasSyntax();
	static MSyntax		cmdSyntax();

	// Do It method
	//
	MStatus		doIt(const MArgList& args) override;

private:
	MStatus parseArgs(const MArgList& args);
	bool	checkArgs()const;
	MStatus Main(const MArgList &args);
	MStatus TryExport(Heightmap	&height_map, MDagPath &dagPath);
	
	MString			m_current_node_name;
	CommnadArgument	m_arg;
};


static bool GetDirName(std::string &out, const char*path) {
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	errno_t err;

	err = _splitpath_s(path, drive, _MAX_DRIVE, dir, _MAX_DIR, fname,_MAX_FNAME, ext, _MAX_EXT);
	if (err != 0){
		return false;
	}
	out.assign(drive);
	out.append(dir);
	return true;
}

static bool GetPolyMeshInfo(PolyMeshInfo&out, MObject& meshNode) {
	MItDependencyGraph dgIt(meshNode, MFn::kPolyMesh, MItDependencyGraph::kUpstream, MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel);
	MObject polyMeshNode = dgIt.currentItem();
	MFnDependencyNode polyFn(polyMeshNode);
	MGlobal::displayInfo(polyFn.name());
	
	double width = 0.0;
	if (polyFn.findPlug("width").getValue(width) != MStatus::kSuccess) {
		return false;
	}
	double height = 0.0;
	if (polyFn.findPlug("height").getValue(height) != MStatus::kSuccess) {
		return false;
	}

	int subdivisionsWidth = 0;
	if (polyFn.findPlug("subdivisionsWidth").getValue(subdivisionsWidth) != MStatus::kSuccess) {
		return false;
	}

	int subdivisionsHeight = 0;
	if (polyFn.findPlug("subdivisionsHeight").getValue(subdivisionsHeight) != MStatus::kSuccess) {
		return false;
	}

	out.m_width = width;
	out.m_height = height;
	out.m_subdivisionsWidth = subdivisionsWidth;
	out.m_subdivisionsHeight = subdivisionsHeight;
	return true;
}

static MStatus BuildVertex(std::vector<unsigned short>&dst, const MDagPath& mdagPath) {
	MStatus stat = MS::kSuccess;
	MItMeshVertex vtxIter(mdagPath, MObject::kNullObj, &stat);
	if (MS::kSuccess != stat) {
		MGlobal::displayError("Failure in MItMeshVertex initialization.");
		return MS::kFailure;
	}

	//・ｽK・ｽ・ｽ・ｽﾈサ・ｽC・ｽY・ｽﾅ予・ｽｷゑｿｽ
	dst.reserve(1024 * 1024);
	dst.clear();

	size_t num_vertex = 0;
	for (; !vtxIter.isDone(); vtxIter.next(), ++num_vertex) {
		MPoint p = vtxIter.position(MSpace::kObject);

		// convert from internal units to the current ui units
		//p.x = MDistance::internalToUI(p.x);
		p.y = MDistance::internalToUI(p.y);
		//p.z = MDistance::internalToUI(p.z);
		dst.push_back(p.y);
	}
	return MS::kSuccess;
}

static void ResizeImage(cv::Mat&dst, size_t dst_width, size_t dst_height,const Heightmap&height_map) {		
	const auto width = height_map.GetImageWidth();
	const auto height = height_map.GetImageHeight();

	cv::Mat src(width, height, CV_16UC1);	
	auto dst_ptr = reinterpret_cast<unsigned short *>(src.ptr());
	memcpy(dst_ptr,&height_map.m_original_image.at(0), width*height*sizeof(unsigned short));
	//dst = src;
	cv::resize(src, dst, cv::Size(dst_width,dst_height), 0, 0, cv::INTER_CUBIC);

}



/////////////////////////////////////////////////////////////////////////////
//	HeigtmapToRaw
/////////////////////////////////////////////////////////////////////////////
HeigtmapToRaw::HeigtmapToRaw() {
	setCommandString("HeigtmapToRaw");
}

HeigtmapToRaw::~HeigtmapToRaw() {

}

void*	HeigtmapToRaw::creator() {
	return new HeigtmapToRaw();
}

MSyntax		HeigtmapToRaw::cmdSyntax() {
	MSyntax syntax;
	syntax.addFlag(FLAG_WIDTH,FLAG_LONG_WIDTH,MSyntax::kUnsigned);
	syntax.addFlag(FLAG_HEIGHT,FLAG_LONG_HEIGHT,MSyntax::kUnsigned);
	syntax.addFlag(FLAG_OUTPUT_DIR, FLAG_LONG_OUTPUT_DIR, MSyntax::kString);
	syntax.enableEdit(false);
	syntax.enableQuery(false);
	return syntax;
}

bool  HeigtmapToRaw::checkArgs()const {	
	if (m_arg.m_width == 0) {
		return false;
	}
	if (m_arg.m_height == 0) {
		return false;
	}
	if (m_arg.m_output_dir.length() == 0) {
		return false;
	}
	return true;
}

MStatus HeigtmapToRaw::parseArgs(const MArgList& args) {
	MStatus status = MS::kSuccess;
	MArgDatabase	argData(syntax(), args, &status);
	if (status != MS::kSuccess) {
		return status;
	}
	if (argData.isFlagSet(FLAG_WIDTH)) {
		if (argData.getFlagArgument(FLAG_WIDTH, 0, m_arg.m_width) != MStatus::kSuccess) {
			return MStatus::kFailure;
		}
	}	
	if (argData.isFlagSet(FLAG_HEIGHT)) {
		if (argData.getFlagArgument(FLAG_HEIGHT, 0, m_arg.m_height) != MStatus::kSuccess) {
			return MStatus::kFailure;
		}
	}
	if (argData.isFlagSet(FLAG_OUTPUT_DIR)) {
		if (argData.getFlagArgument(FLAG_OUTPUT_DIR, 0, m_arg.m_output_dir) != MStatus::kSuccess) {
			return MStatus::kFailure;
		}
	}
	
	return MS::kSuccess;
}

MStatus HeigtmapToRaw::doIt(const MArgList &arg_list) {
	MStatus	result;	
	try {
		result = Main(arg_list);
	}catch (...){
		result = MS::kFailure;
	}	
	return result;
}

MStatus HeigtmapToRaw::TryExport(Heightmap	&height_map, MDagPath &dagPath) {
	{
		MObject meshNode = dagPath.node();
		if (!GetPolyMeshInfo(height_map.m_poly_mesh, meshNode)) {
			return MStatus::kNotFound;
		}
	}

	std::string output_dir(m_arg.m_output_dir.asChar());
	output_dir.append("\\");
	MakeSureDirectoryPathExists(output_dir.c_str());
	
	BuildVertex(height_map.m_original_image, dagPath);
	{
		cv::Mat result_image;

		ResizeImage(result_image, m_arg.m_width, m_arg.m_height, height_map);
		std::string output_filename(output_dir + height_map.m_node_name + ".png");
		cv::imwrite(output_filename, result_image);
	}
	return MStatus::kSuccess;
}

MStatus HeigtmapToRaw::Main(const MArgList &args)
{
	{
		MStatus	status;
		status = parseArgs(args);
		if(status != MStatus::kSuccess) {
			return status;
		}
		if (! checkArgs()) {
			return MStatus::kFailure;
		}
	}

	MSelectionList slist;
	MGlobal::getActiveSelectionList(slist);
	MItSelectionList iter(slist);

	if (iter.isDone()) {
		MGlobal::displayError("Error: Nothing is selected.");
		return MS::kFailure;
	}

	
	MStatus status;
	// We will need to interate over a selected node's heirarchy 
	// in the case where shapes are grouped, and the group is selected.
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
	for (; !iter.isDone(); iter.next())
	{
		MDagPath objectPath;
		// get the selected node
		status = iter.getDagPath(objectPath);

		// reset iterator's root node to be the selected node.
		status = dagIterator.reset(objectPath.node(), MItDag::kDepthFirst, MFn::kInvalid);				
		m_current_node_name = objectPath.partialPathName();

		// DAG iteration beginning at at selected node
		for (; !dagIterator.isDone(); dagIterator.next())
		{
			MDagPath dagPath;
			status = dagIterator.getPath(dagPath);

			if (!status) {
				MGlobal::displayInfo("Failure getting DAG path.");
				return MS::kFailure;
			}
			MFnDagNode dagNode(dagPath, &status);
			if (dagNode.isIntermediateObject())
			{
				continue;
			}
			else if (dagPath.hasFn(MFn::kMesh)) {				
				Heightmap	height_map;							
				
				height_map.m_node_name.assign(m_current_node_name.asChar());
				TryExport(height_map, dagPath);
				continue;
			}
		}
	}
	return MS::kSuccess;
}



/////////////////////////////////////////////////////////////////////////////
//	Initialize.
/////////////////////////////////////////////////////////////////////////////
MStatus initializePlugin(MObject _obj)
{
	MFnPlugin	plugin(_obj, "(C)Ohtorii", "0.2.0","4.5");
	MStatus		stat;
	stat = plugin.registerCommand(COMMAND_NAME, HeigtmapToRaw::creator, HeigtmapToRaw::cmdSyntax);
	if (!stat) {
		stat.perror("registerCommand");
	}
	return stat;												
}																

MStatus uninitializePlugin(MObject _obj)						
{																
	MFnPlugin	plugin(_obj);									
	MStatus		stat;											
	stat = plugin.deregisterCommand(COMMAND_NAME);
	if (!stat) {
		stat.perror("deregisterCommand");
	}
	return stat;												
}
