import FreeCAD
import importDXF
import importSVG
import argparse

def dxf2svg(dxf, svg):
    importDXF.open(dxf)
    doc = FreeCAD.activeDocument()

    objects = doc.findObjects()
    importSVG.export(objects, svg)

if __name__ == '__main__':
    p = argparse.ArgumentParser(description='Convert .dxf to .svg with the help of FreeCAD')
    p.add_argument('dxf')
    p.add_argument('svg')
    args = p.parse_args()
    dxf2svg(args.dxf, args.svg)

