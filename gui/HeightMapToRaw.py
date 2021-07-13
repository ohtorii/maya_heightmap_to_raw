# -*- coding: utf-8 -*-
from __future__ import unicode_literals
from maya.app.general.mayaMixin import MayaQWidgetBaseMixin

import os
import xml.etree.ElementTree as ET

import maya.mel as mm

try:
    from PySide2.QtWidgets import QMainWindow, QApplication, QDialog, QFileDialog
    from PySide2.QtUiTools import QUiLoader
    from PySide2 import QtCore
except ImportError:
    from PySide.QtGui import QMainWindow, QApplication, QDialog, QFileDialog
    from PySide.QtUiTools import QUiLoader
    from PySide import QtCore


# =============================================================================
class _MainWindow(MayaQWidgetBaseMixin, QMainWindow):
    def __init__(self, parent=None):
        super(_MainWindow, self).__init__(parent)
        ui_file = os.path.join(os.path.dirname(__file__), 'HeightMapToRaw.ui')
        self.ui = gui(self, ui_file, 'HeightMapToRaw: version 0.2.0')
        # シグナルの登録
        self.ui.output_dir_select_.clicked.connect(self.output_dir_select)
        self.ui.export_.clicked.connect(self.export)
    
    def output_dir_select(self):
        dir=QFileDialog.getExistingDirectory(self, "Select Directory")
        if dir:
            w = self.ui.output_dir_
            w.selectAll()
            w.backspace()
            w.insert(dir)
        return
        
    def export(self):
        w=self.ui.width_.value()
        h=self.ui.height_.value()
        output_dir=self.ui.output_dir_.text().replace("\\","\\\\")
        cmd = 'HeigtMapToRaw -width %d -height %d -output_dir "%s"' % (w,h,output_dir)
        print(cmd)
        mm.eval(cmd)




def gui(qt, ui_file, title):
    #MayaPythonGuiDemo
    # uiファイルのロード
    loader = QUiLoader()
    ui = loader.load(ui_file)
    size = get_ui_size(ui_file)

    try:
        qt.setCentralWidget(ui)
    except AttributeError:
        layout = QVBoxLayout()
        layout.addWidget(ui)
        qt.setLayout(layout)
        size[0] += 20
        size[1] += 20

    # ウィンドウのタイトルとサイズを設定
    qt.setWindowTitle(title)
    qt.setFixedSize(size[0], size[1])

    return ui

def get_ui_size(ui_file):
    #MayaPythonGuiDemo
    # uiファイルからウィンドウサイズを取得
    tree = ET.parse(ui_file)
    elem = tree.getroot()
    width  = elem.findtext(".//width")
    height = elem.findtext(".//height")
    return [int(width), int(height)]
    
def main():
    #MayaPythonGuiDemo
    QApplication.instance()
    gui = _MainWindow()
    gui.show()


if __name__ == '__main__':
    main()

