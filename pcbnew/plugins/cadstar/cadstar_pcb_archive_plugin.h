/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2020 Roberto Fernandez Bautista <roberto.fer.bau@gmail.com>
 * Copyright (C) 2020 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file cadstar_pcb_archive_plugin.h
 * @brief Pcbnew PLUGIN for CADSTAR PCB Archive (*.cpa) format: an ASCII format
 *        based on S-expressions.
 */

#ifndef CADSTAR_ARCHIVE_PLUGIN_H_
#define CADSTAR_ARCHIVE_PLUGIN_H_


#include <io_mgr.h>

class CADSTAR_PCB_ARCHIVE_PLUGIN : public PLUGIN
{
public:
    // -----<PUBLIC PLUGIN API>--------------------------------------------------

    const wxString PluginName() const override;

    BOARD* Load( const wxString& aFileName, BOARD* aAppendToMe,
            const PROPERTIES* aProperties = NULL ) override;

    const wxString GetFileExtension() const override;

    long long GetLibraryTimestamp( const wxString& aLibraryPath ) const override
    {
        // No support for libraries....
        return 0;
    }

    // -----</PUBLIC PLUGIN API>-------------------------------------------------

    CADSTAR_PCB_ARCHIVE_PLUGIN();
    ~CADSTAR_PCB_ARCHIVE_PLUGIN();

private:
    const PROPERTIES* m_props;
    BOARD*            m_board;
};

#endif // CADSTAR_ARCHIVE_PLUGIN_H_
