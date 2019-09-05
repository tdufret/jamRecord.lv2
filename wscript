#! /usr/bin/env python
# encoding: utf-8

# Variables for 'waf dist'
APPNAME = 'jamRecord.lv2'
VERSION = '0.0.2'

# Mandatory variables
top = '.'
out = 'build'


def options(ctx):
    ctx.load('compiler_c')
    ctx.load('lv2', tooldir='.')

    ctx.add_option('--debug', action='store', default=False, help='enable debug mode')


def configure(ctx):
    print('Configuring %s v%s project.' %(APPNAME,VERSION))
    print('Debug mode is %s' %ctx.options.debug)

    ctx.load('compiler_c')
    ctx.load('lv2', tooldir='.')
