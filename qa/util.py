import os, random
from conf import *

def str_random_generate (n):
    c = ""
    x = int (random.random() * 100)
    y = int (random.random() * 100)
    z = int (random.random() * 100)

    for i in xrange(n):
        x = (171 * x) % 30269
        y = (172 * y) % 30307
        z = (170 * z) % 30323
        c += chr(32 + int((x/30269.0 + y/30307.0 + z/30323.0) * 1000 % 95))

    return c

def letters_random_generate (n):
    c = ""
    x = int (random.random() * 100)
    y = int (random.random() * 100)
    z = int (random.random() * 100)

    for i in xrange(n):
        x = (171 * x) % 30269
        y = (172 * y) % 30307
        z = (170 * z) % 30323
        if z%2:
            c += chr(65 + int((x/30269.0 + y/30307.0 + z/30323.0) * 1000 % 25))
        else:
            c += chr(97 + int((x/30269.0 + y/30307.0 + z/30323.0) * 1000 % 25))
            
    return c


str_buf     = ""
letters_buf = ""

def letters_random (n):
    global letters_buf

    letters_len = len(letters_buf)
    if letters_len == 0:
        letters_random_generate (1000)
        letters_len = len(letters_buf)

    offset = random.randint (0, letters_len)

    if letters_len - offset > n:
        return letters_buf[offset:offset+n]

    tmp = letters_random_generate (n - (letters_len - offset))
    letters_buf += tmp

    return letters_buf[offset:]


def str_random (n):
    global str_buf

    str_len = len(str_buf)
    if str_len == 0:
        str_random_generate (1000)
        str_len = len(str_buf)

    offset = random.randint (0, str_len)

    if str_len - offset > n:
        return str_buf[offset:offset+n]

    tmp = str_random_generate (n - (str_len - offset))
    str_buf += tmp
    return str_buf[offset:]


def check_php_interpreter (fullpath):
    f = os.popen ("%s -v" % (fullpath))
    all = reduce (lambda x,y: x+y, f.readlines())
    f.close()
    return "cgi-fcgi" in all

__php_ref = None
def look_for_php():    
    global __php_ref

    if __php_ref != None:
        return __php_ref
    
    if PHPCGI_PATH != "auto":
        __php_ref = PHPCGI_PATH
        if not check_php_interpreter(__php_ref):
            print "%s doesn't support fcgi"
        return __php_ref

    for p in PHP_DIRS:
        for n in PHP_NAMES:
            php = os.path.join(p,n)
            if os.path.exists(php):
                __php_ref = php
                if not check_php_interpreter(__php_ref):
                    print "%s doesn't support fcgi"
                return php

    print "ERROR: PHP interpreter not found"
    __php_ref = ''
    return __php_ref


__python_ref = None
def look_for_python():
    global __python_ref

    if __python_ref != None:
        return __python_ref    

    if PYTHON_PATH != "auto":
        __python_ref = PYTHON_PATH
        return __python_ref

    for p in PYTHON_DIRS:
        for n in PYTHON_NAMES:
            py = os.path.join(p,n)
            if os.path.exists(py):
                __python_ref = py
                return py

    print "ERROR: Python interpreter not found"
    __python_ref = ''
    return __python_ref


def print_key (key, val):
    print "%10s: %s" % (key, val)
