

/*
 Copyright (c) 2008-2022, Benoit AUTHEMAN All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author or Destrat.io nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//-----------------------------------------------------------------------------
// This file is a part of the QuickQanava software library.
// \file	EdgeStraightPath.qml
// \author	benoit@destrat.io
// \date	2022 10 02
//-----------------------------------------------------------------------------
import QtQuick 2.7
import QtQuick.Shapes 1.0

ShapePath {
    id: edgeShapePath

    // Set in EdgeTemplate.qml createObject() from global qanEdgeStraightPathComponent
    property var edgeTemplate: undefined
    property var edgeItem: edgeTemplate.edgeItem

    // Color (Gradient)
    property color startColor: Qt.rgba(0., 0., 0., 1.)
    property color endColor: Qt.rgba(0., 0., 0., 1.)
    property double startPosition: 0.0
    property double endPosition: 1.0

    // Rec Position
    property point vecStart: calcVec(edgeItem.p1, edgeItem.p2)
    property point vecEnd: Qt.point(-vecStart.x, -vecStart.y)

    // RecWidth (Style)
    property double recStrength: 1.5

    startX: edgeItem.p1.x
    startY: edgeItem.p1.y
    capStyle: ShapePath.FlatCap
    strokeWidth: edgeItem && edgeItem.style ? edgeItem.style.lineWidth : 2
    fillGradient: LinearGradient {
        x1: edgeItem.p1.x
        y1: edgeItem.p1.y
        x2: edgeItem.p2.x
        y2: edgeItem.p2.y

        GradientStop {
            position: startPosition
            color: edgeShapePath.startColor
        }
        GradientStop {
            position: endPosition
            color: edgeShapePath.endColor
        }
    }
    strokeColor: Qt.rgba(0, 0, 0, 0)
    strokeStyle: edgeTemplate.dashed
    dashPattern: edgeItem
                 && edgeItem.style ? edgeItem.style.dashPattern : [2, 2]
    PathLine {
        x: addVec(orthoL(vecStart), edgeItem.p1).x
        y: addVec(orthoL(vecStart), edgeItem.p1).y
    }
    PathLine {
        x: addVec(orthoR(vecEnd), edgeItem.p2).x
        y: addVec(orthoR(vecEnd), edgeItem.p2).y
    }
    PathLine {
        x: addVec(orthoL(vecEnd), edgeItem.p2).x
        y: addVec(orthoL(vecEnd), edgeItem.p2).y
    }
    PathLine {
        x: addVec(orthoR(vecStart), edgeItem.p1).x
        y: addVec(orthoR(vecStart), edgeItem.p1).y
    }
    PathLine {
        x: addVec(orthoL(vecStart), edgeItem.p1).x
        y: addVec(orthoL(vecStart), edgeItem.p1).y
    }

    function calcVec(p1, p2) {
        var vecX = p2.x - p1.x
        var vecY = p2.y - p1.y
        return Qt.point(vecX, vecY)
    }
    function addVec(p1, p2) {
        var vecX = p2.x + p1.x
        var vecY = p2.y + p1.y
        return Qt.point(vecX, vecY)
    }
    function orthoL(vec) {
        return normalize(Qt.point(-vec.y, vec.x))
    }
    function orthoR(vec) {
        return normalize(Qt.point(vec.y, -vec.x))
    }

    function normalize(vec) {
        var amount = (1 / Math.sqrt(Math.pow(vec.x,
                                             2) + Math.pow(vec.y,
                                                           2))) * recStrength
        return Qt.point(vec.x * amount, vec.y * amount)
    }
}
