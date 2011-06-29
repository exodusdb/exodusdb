#!/usr/bin/env python

"""
setup.py file for SWIG exodus
"""

from distutils.core import setup, Extension


exodus_module = Extension('_exodus',
                           sources=['exodus_wrap.cxx'],
                           libraries=['exodus']
                           )

setup (name    = 'exodus',
       version = '11.6',
       author       = "Steve Bush",
       author_email = "steve.bush@neosys.com",
       license      = "BSD",
       url          = "http://code.google.com/p/exodusdb/",
       description      = """Exodus Python module for Multivalue Database Programming""",
       long_description = """Exodus Python module for Multivalue Database Programming""",
       ext_modules  = [exodus_module],
       py_modules   = ["exodus"],
       keywords="Database, Multivalue, NoSQL, Northgate, Universe, OpenQM, OpenInsight, Cache, U2, AdvancedRevelation, Pick, JBase, Reality",
       )

