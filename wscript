#! /usr/bin/env python
# encoding: utf-8

from waflib.extras import autowaf as autowaf

# Variables for 'waf dist'
APPNAME = 'jamRecord.lv2'
VERSION = '1.0.0'

# Mandatory variables
top = '.'
out = 'build'


def options(opt):
    opt.load('compiler_c')
    opt.load('lv2')
    autowaf.set_options(opt)

    
def configure(ctx):
    print('Configuring the project %s (v%s).' %(APPNAME, VERSION))
    print('→ prefix is ' + ctx.options.prefix)

    ctx.load('compiler_c')
    ctx.load('lv2')
