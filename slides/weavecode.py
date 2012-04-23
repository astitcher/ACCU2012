#!/usr/bin/env python

## For cgi.escape
import cgi
import re

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

## Read source looking for every line like [[[[<mark> and return all the <mark>s
def findMarks(s):
    pattern = re.compile(r'.*\[\[\[\[(.*?)\s.*')
    return [m.group(1) for m in filter(lambda x: x != None, [pattern.match(l) for l in s])]

## Read file look for marks then read each segment escaped into a dictionary
def scanSource(d, f):
    source = readSource(f)
    marks = findMarks(source)
    for m in marks:
        print "Found: ", m
        d[m] = escapeSource(clipSource(source, m))

## Read template for lines starting with [[[[ and ending ]]]]
## writing out non matching lines as-is and substituting the matched lines
## with the segments already read
def expandTemplate(d, template, output):
    pattern = re.compile(r'.*\[\[\[\[(.*)\]\]\]\].*')
    with open(output, 'w') as wf:
        for l in open(template):
            match = pattern.match(l)
            if match:
                mark = match.group(1)
                if mark in d:
                    print "Substituting: ", mark
                    wf.writelines(d[mark])
                else:
                    print "Segment not known: ", mark
                    wf.write(l)
            else:
                wf.write(l)

