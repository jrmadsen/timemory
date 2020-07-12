# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# sys.path.insert(0, os.path.abspath('.'))
import os
import sys
import shutil
import subprocess as sp

from recommonmark.transform import AutoStructify
from recommonmark.parser import CommonMarkParser
import recommonmark

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
sys.path.insert(0, os.path.abspath('..'))


def install(package):
    sp.call([sys.executable, "-m", "pip", "install", package])


# Check if we're running on Read the Docs' servers
read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'


# -- Project information -----------------------------------------------------
project = 'timemory'
copyright = '2020,  The Regents of the University of California'
author = 'Jonathan R. Madsen'

version = open(os.path.join('..', 'VERSION')).read().strip()
# The full version, including alpha/beta/rc tags
release = version

_docdir = os.path.realpath(os.getcwd())
_srcdir = os.path.realpath(os.path.join(os.getcwd(), ".."))
_bindir = os.path.realpath(os.path.join(os.getcwd(), "build-timemory"))
_doxbin = os.path.realpath(os.path.join(_bindir, "doc"))
_doxdir = os.path.realpath(os.path.join(
    _docdir, "_build", "html", "doxygen-docs"))
_xmldir = os.path.realpath(os.path.join(_docdir, "doxygen-xml"))
_sitedir = os.path.realpath(os.path.join(os.getcwd(), "..", "site"))

if not os.path.exists(_bindir):
    os.makedirs(_bindir)

os.chdir(_bindir)
sp.run(["cmake",
        "-DTIMEMORY_BUILD_DOCS=ON", "-DENABLE_DOXYGEN_XML_DOCS=ON",
        "-DENABLE_DOXYGEN_HTML_DOCS=ON", "-DENABLE_DOXYGEN_LATEX_DOCS=OFF",
        "-DENABLE_DOXYGEN_MAN_DOCS=OFF", "-DTIMEMORY_BUILD_KOKKOS_TOOLS=ON",
        _srcdir])
sp.run(["cmake", "--build", os.getcwd(), "--target", "doc"])

if os.path.exists(_doxdir):
    shutil.rmtree(_doxdir)

if os.path.exists(_xmldir):
    shutil.rmtree(_xmldir)

shutil.copytree(os.path.join(_doxbin, "html"), _doxdir)
shutil.copytree(os.path.join(_doxbin, "xml"), _xmldir)
shutil.copyfile(
    os.path.join(_bindir, "doc", "Doxyfile.timemory"),
    os.path.join(_docdir, "Doxyfile.timemory"))
shutil.rmtree(_bindir)
for t in ["timem", "timemory-run", "timemory-mpip", "timemory-ompt", "timemory-ncclp",
          "timemory-avail", "timemory-jump", "timemory-stubs", "kokkos-connector"]:
    shutil.copyfile(
        os.path.join(_srcdir, "source", "tools", t, "README.md"),
        os.path.join(_docdir, "tools", t, "README.md"))

shutil.copyfile(os.path.join(_srcdir, "source", "python", "README.md"),
                os.path.join(_docdir, "api", "python.md"))

# install('mkdocs-cinder')
# install('mkdocs-inspired')
# os.chdir(_srcdir)
# sp.run(["mkdocs", "build"])
# remove the index.html generated by mkdocs
# os.remove(os.path.join(_sitedir, "index.html"))
# os.chdir(_docdir)
# html_extra_path = [_doxdir]

os.chdir(_docdir)

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.doctest',
    'sphinx.ext.todo',
    'sphinx.ext.viewcode',
    'sphinx.ext.githubpages',
    'sphinx.ext.mathjax',
    'sphinx.ext.autosummary',
    'sphinx.ext.napoleon',
    'sphinx_markdown_tables',
    'recommonmark',
    'breathe',
]

source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

source_parsers = {
    '.md': CommonMarkParser
}

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The master toctree document.
master_doc = 'index'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

default_role = None

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Breathe Configuration
breathe_projects = {}
breathe_default_project = project
breathe_projects[project] = 'doxygen-xml'
breathe_projects_source = {
    "auto": ("../source", ["library.cpp", "trace.cpp", "timemory_c.c", "timemory_c.cpp"])
}

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'


# app setup hook
def setup(app):
    app.add_config_value('recommonmark_config', {
        'auto_toc_tree_section': 'Contents',
        'enable_eval_rst': True,
        'enable_auto_doc_ref': False,
    }, True)
    app.add_transform(AutoStructify)
