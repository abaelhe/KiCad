# This program source code file is part of KiCad, a free EDA CAD application.
#
# Copyright (C) 2014-2019 KiCad Developers, see AUTHORS.txt for contributors.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you may find one here:
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
# or you may search the http://www.gnu.org website for the version 2 license,
# or you may write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


if( KICAD_TEST_XML_OUTPUT )
    # To do this, you will need xmlrunner
    set( PYTEST_ARGS_QACLI --junitxml=${CMAKE_CURRENT_BINARY_DIR}/qacli.junit-results.xml )
endif()

# avoid formatting this otherwise you will introduce whitespace into the PATH string
    set( QA_PATH_REPLACEMENT
"PATH=${CMAKE_BINARY_DIR}/kicad/;\
${CMAKE_BINARY_DIR}/pcbnew/;\
${CMAKE_BINARY_DIR}/eeschema/;\
$ENV{PATH};" )

if (UNIX)
  string(REPLACE ";" ":" QA_PATH_REPLACEMENT "${QA_PATH_REPLACEMENT}")
endif()


add_test(NAME qa_cli
    COMMAND ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/cli ${PYTEST_ARGS_QACLI}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set_tests_properties( qa_cli
    PROPERTIES
        ENVIRONMENT "${QA_PATH_REPLACEMENT};KICAD_RUN_FROM_BUILD_DIR=1;PYTHONPATH=${CMAKE_BINARY_DIR}/pcbnew;${PYTHON_QA_PATH}"
)
