UT_UTF8String sMathSVGScript = " \
/* \r\n \
MIT License\r\n \
Copyright (c) 2006 Sam Ruby \r\n \
\r\n \
Permission is hereby granted, free of charge, to any person obtaining a copy\r\n \
of this software and associated documentation files (the 'Software'), to deal\r\n \
in the Software without restriction, including without limitation the rights\r\n \
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\r\n \
copies of the Software, and to permit persons to whom the Software is\r\n \
furnished to do so, subject to the following conditions: \r\n \
\r\n \
The above copyright notice and this permission notice shall be included in all\r\n \
copies or substantial portions of the Software.\r\n \
THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\r\n \
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, \r\n \
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\r\n \
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\r\n \
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\r\n \
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\r\n \
SOFTWARE.\r\n \
*/ \r\n \
\r\n \
window.onload = function() {\r\n \
  var MathML = {\r\n \
    // Jacques Distler subset of Presentational MathML\r\n \
    root: 'math',\r\n \
    ns: 'http://www.w3.org/1998/Math/MathML',\r\n \
    elements: ['maction', 'math', 'merror', 'mfrac', 'mi',\r\n \
      'mmultiscripts', 'mn', 'mo', 'mover', 'mpadded', 'mphantom',\r\n \
      'mprescripts', 'mroot', 'mrow', 'mspace', 'msqrt', 'mstyle', 'msub',\r\n \
      'msubsup', 'msup', 'mtable', 'mtd', 'mtext', 'mtr', 'munder',\r\n \
      'munderover', 'none'], \r\n \
    attributes: ['actiontype', 'align', 'columnalign', 'columnalign', \r\n \
      'columnalign', 'columnlines', 'columnspacing', 'columnspan', 'depth', \r\n \
      'display', 'displaystyle', 'equalcolumns', 'equalrows', 'fence', \r\n \
      'fontstyle', 'fontweight', 'frame', 'height', 'linethickness', 'lspace', \r\n \
      'mathbackground', 'mathcolor', 'mathvariant', 'mathvariant', 'maxsize',  \r\n \
      'minsize', 'other', 'rowalign', 'rowalign', 'rowalign', 'rowlines', \r\n \
      'rowspacing', 'rowspan', 'rspace', 'scriptlevel', 'selection',\r\n \
      'separator', 'stretchy', 'width', 'width'] \r\n \
  } \r\n \
 \r\n \
  var SVG = { \r\n \
    // svgtiny + class + opacity + offset + style \r\n \
    root: 'svg', \r\n \
    ns: 'http://www.w3.org/2000/svg',\r\n \
    elements: ['a', 'animate', 'animateColor', 'animateMotion', \r\n \
      'animateTransform', 'circle', 'defs', 'desc', 'ellipse', 'font-face', \r\n \
      'font-face-name', 'font-face-src', 'g', 'glyph', 'hkern', 'image', \r\n \
      'linearGradient', 'line', 'metadata', 'missing-glyph', 'mpath', 'path', \r\n \
      'polygon', 'polyline', 'radialGradient', 'rect', 'set', 'stop',\r\n \
      'switch', 'text', 'title', 'use'], \r\n \
    attributes: ['accent-height', 'accumulate', 'additive', 'alphabetic', \r\n \
      'arabic-form', 'ascent', 'attributeName', 'attributeType', \r\n \
      'baseProfile', 'bbox', 'begin', 'by', 'calcMode', 'cap-height', \r\n \
      'class', 'color', 'color-rendering', 'content', 'cx', 'cy', 'd', \r\n \
      'descent', 'display', 'dur', 'end', 'fill', 'fill-rule', 'font-family', \r\n \
      'font-size', 'font-stretch', 'font-style', 'font-variant', \r\n \
      'font-weight', 'from', 'fx', 'fy', 'g1', 'g2', 'glyph-name', 'hanging', \r\n \
      'height', 'horiz-adv-x', 'horiz-origin-x', 'id', 'ideographic', 'k', \r\n \
      'keyPoints', 'keySplines', 'keyTimes', 'lang', 'mathematical', 'max', \r\n \
      'min', 'name', 'offset', 'opacity', 'origin', 'overline-position',\r\n \
      'overline-thickness', 'panose-1', 'path', 'pathLength', 'points',\r\n \
      'preserveAspectRatio', 'r', 'repeatCount', 'repeatDur',\r\n \
      'requiredExtensions', 'requiredFeatures', 'restart', 'rotate', 'rx',\r\n \
      'ry', 'slope', 'stemh', 'stemv', 'stop-color', 'stop-opacity',\r\n \
      'strikethrough-position', 'strikethrough-thickness', 'stroke',\r\n \
      'stroke-dasharray', 'stroke-dashoffset', 'stroke-linecap',\r\n \
      'stroke-linejoin', 'stroke-miterlimit', 'stroke-width', 'style',\r\n \
      'systemLanguage', 'target', 'text-anchor', 'to', 'transform', 'type',\r\n \
      'u1', 'u2', 'underline-position', 'underline-thickness', 'unicode',\r\n \
      'unicode-range', 'units-per-em', 'values', 'version', 'viewBox',\r\n \
      'visibility', 'width', 'widths', 'x', 'x-height', 'x1', 'x2',\r\n \
      'y', 'y1', 'y2', 'zoomAndPan'] \r\n \
  } \r\n \
 \r\n \
  // clone a DOM subtree \r\n \
  function deepcopy(module, source, dest, nsmap) { \r\n \
    // copy attributes \r\n \
    for (var i=0; i<source.attributes.length; i++) { \r\n \
      var oldattr = source.attributes[i];\r\n \
      var colon = oldattr.name.indexOf(':');\r\n \
      if (colon == -1) {\r\n \
        for (var j=0; j<module.attributes.length; j++) {\r\n \
          if (module.attributes[j].toLowerCase() != oldattr.name) continue;\r\n \
          dest.setAttribute(module.attributes[j], oldattr.value);\r\n \
          break;\r\n \
        }\r\n \
      } else { // namespace prefixed attribute\r\n \
        var prefix = oldattr.name.slice(0,colon);\r\n \
        var name = oldattr.name.slice(colon+1);\r\n \
        if (prefix == 'xmlns') {\r\n \
          var oldmap = nsmap;\r\n \
          nsmap = {};\r\n \
          for (var property in oldmap) nsmap[property] = oldmap[property];\r\n \
          nsmap[name] = oldattr.value;\r\n \
        } else {\r\n \
          for (var ns in nsmap) {\r\n \
            if (ns == prefix) {\r\n \
              dest.setAttributeNS(nsmap[prefix], name, oldattr.value);\r\n \
            }\r\n \
          }\r\n \
        }\r\n \
      }\r\n \
    }\r\n \
\r\n \
    // copy children\r\n \
    for (var i=0; i<source.childNodes.length; i++) {\r\n \
      var oldchild = source.childNodes[i];\r\n \
      if (oldchild.nodeType == 1) { // element\r\n \
        for (var j=0; j<module.elements.length; j++) {\r\n \
          if (module.elements[j].toUpperCase() != oldchild.nodeName) continue;\r\n \
          var newchild = document.createElementNS(module.ns,module.elements[j]);\r\n \
          deepcopy(module, oldchild, newchild, nsmap);\r\n \
          dest.appendChild(newchild);\r\n \
          break;\r\n \
        }\r\n \
      } else if (oldchild.nodeType == 3) { // text\r\n \
        var newchild = document.createTextNode(oldchild.nodeValue);\r\n \
        dest.appendChild(newchild); \r\n \
      } \r\n \
    } \r\n \
  } \r\n \
\r\n \
  // copy modules into their appropriate namespaces\r\n \
  var modules = [MathML, SVG];\r\n \
  for (var i=0; i<modules.length; i++) { \r\n \
    var module = modules[i];\r\n \
    var roots = document.getElementsByTagName(module.root); \r\n \
    for (var j=0; j<roots.length; j++) { \r\n \
      var source = roots[j]; \r\n \
      if (document.createElementNS) { \r\n \
        if (source.__proto__ != HTMLUnknownElement.prototype) continue; \r\n \
        if (source.getAttribute('xmlns') != module.ns) continue; \r\n \
        var dest = document.createElementNS(module.ns, module.root); \r\n \
        deepcopy(module, source, dest, {}); \r\n \
        source.parentNode.insertBefore(dest, source); \r\n \
        source.parentNode.removeChild(source); \r\n \
      } else {  \r\n \
        // fallback browsers that don't support DOM namespaces \r\n \
        var img = document.createElement('img'); \r\n \
        img.src = module.root + ' image'; \r\n \
        img.title = module.root + ' image'; \r\n \
        source.parentNode.insertBefore(img, source) ;\r\n \
      } \r\n \
    } \r\n \
  } \r\n \
} \r\n";

