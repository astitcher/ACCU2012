#!/usr/bin/env python -e

## For cgi.escape
import cgi

## Read in a source file into an array of lines
def readSource(f):
    return [l for l in open(f)]
    

## Search source for the segment we want starting with
## '[[[[<mark>' and ending with ']]]]<mark>' anywhere on 
## a line - don't include the lines themselves
def clipSource(s, mark):
    # Look for first mark
    for i in range(len(s)):
        if s[i].find('[[[['+mark) >= 0:
            break
    else:
        raise NameError("Segment start not found " + mark)
    # Look from there for next mark
    for j in range(i,len(s)):
        if s[j].find(']]]]'+mark) >= 0:
            break
    else:
        raise NameError("Segment end not found " + mark)
    return s[i+1:j]

## Escape every line
def escapeSource(s):
    return [cgi.escape(l) for l in s]



## Read template for lines starting with [[[[ and ending ]]]]
