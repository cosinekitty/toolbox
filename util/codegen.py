#!/usr/bin/env python3
import sys
from svgpanel import *

TOOLBOX_FONT_FILENAME = 'Quicksand-Light.ttf'
TOOLBOX_PANEL_COLOR = '#a06de4'
TOOLBOX_BORDER_COLOR = '#5021d4'

MODEL_NAME_POINTS = 22.0
MODEL_NAME_STYLE    = 'display:inline;stroke:#000000;stroke-width:0.35;stroke-linecap:round;stroke-linejoin:bevel'


def Print(message:str) -> int:
    print('codegen.py:', message)
    return 0


def Save(panel:BasePanel, filename:str) -> int:
    panel.save(filename)
    return 0


def MakeBorder(hpWidth:int, mmHeight:float = PANEL_HEIGHT_MM) -> BorderRect:
    return BorderRect(hpWidth, TOOLBOX_PANEL_COLOR, TOOLBOX_BORDER_COLOR, mmHeight)


def ModelNameElement(name:str, font:Font, panel:Panel) -> TextPath:
    modelTextItem = TextItem('modscan', font, MODEL_NAME_POINTS)
    (mdx, _) = modelTextItem.measure()
    x1 = (panel.mmWidth - mdx) / 2
    y1 = 0.2
    modelTextPath = TextPath(modelTextItem, x1, y1)
    modelTextPath.setAttrib('style', MODEL_NAME_STYLE)
    return modelTextPath


def GenerateModuleScannerPanel() -> int:
    panelWidth = 8
    svgFileName = '../res/modscan.svg'
    panel = Panel(panelWidth)
    pl = Element('g', 'PanelLayer')
    panel.append(pl)
    with Font(TOOLBOX_FONT_FILENAME) as font:
        pl.append(MakeBorder(panelWidth))
        pl.append(ModelNameElement('modscan', font, panel))
    return panel.save(svgFileName)


if __name__ == '__main__':
    sys.exit(
        GenerateModuleScannerPanel() or
        Print('SUCCESS')
    )
