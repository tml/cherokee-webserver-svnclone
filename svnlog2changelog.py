#!/usr/bin/env python

##
## Cherokee SVNlog2Changelog script
##
## Copyright: Alvaro Lopez Ortega <alvaro@alobbs.com>
## Licensed: GPL v2
##

import time
from sys import stdin
import xml.dom.minidom

DEVELOPERS = {
    'alo' : "Alvaro Lopez Ortega  <alvaro@octality.com>"
}

def get_text (nodelist):
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            return node.data

def entry_get_val (entry, key):
    dom_element = entry.getElementsByTagName(key)[0]
    nodelist    = dom_element.childNodes
    return get_text(nodelist)

def reformat_msg (msg):
    if not msg:
        return ''

    txts = []
    for l in msg.split('\n'):
        if not txts:
            txts.append('\t* %s'%(l))
        else:
            txts.append('\t%s'%(l))
    return '\n'.join(txts)

def render_paths (paths):
    txt = ''
    for entry in log.getElementsByTagName('path'):
        action = entry.getAttribute('action')
        txt += "\t%s %s\n"%(action, get_text(entry.childNodes))
    return txt        

dom = xml.dom.minidom.parseString (stdin.read())
log = dom.getElementsByTagName('log')[0]

for entry in log.getElementsByTagName('logentry'):    
    revision = entry.getAttribute('revision')
    date     = entry_get_val (entry, 'date').split('T')[0]
    time     = entry_get_val (entry, 'date').split('T')[1].split('.')[0]
    dev      = entry_get_val (entry, 'author')
    msg      = reformat_msg(entry_get_val (entry, 'msg'))
    author   = DEVELOPERS[dev]
    paths    = render_paths(entry_get_val (entry, 'paths'))
    
    print "%s  %s" % (date, author)
    print " "*12 + "SVN: r%s, %s - %s" % (revision, dev, time)
    if msg:
        print
        print msg,
    if paths:
        print
        print paths

