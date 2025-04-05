# ~/exodus/pygment/setup.py
from setuptools import setup
setup(
    name='exodus_lexers',
    py_modules=['exodus_cpp'],
    entry_points={
        'pygments.lexers': [
            'exoduscpp = exodus_cpp:ExodusCppLexer',
        ]
    }
)