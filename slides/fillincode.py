#!/usr/bin/env python
from weavecode import *

segs = {}

# Scan source files for segments
scanSource(segs, '../AlgebraicTypesInC++/intlist_UnionSolution.cpp')
scanSource(segs, '../AlgebraicTypesInC++/intlist.cpp')
scanSource(segs, '../AlgebraicTypesInC++/TemplateSol.cpp')
scanSource(segs, '../AlgebraicTypesInC++/intlist.ml')
scanSource(segs, '../AlgebraicTypesInC++/expression.ml')
scanSource(segs, '../AlgebraicTypesInC++/simple.ml')


# Create substituted putput
expandTemplate(segs, 'algebraic-template.html', 'algebraic.html')

