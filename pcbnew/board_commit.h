/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * Copyright (C) 2016-2023 KiCad Developers, see AUTHORS.txt for contributors.
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef BOARD_COMMIT_H
#define BOARD_COMMIT_H

#include <commit.h>

class BOARD_ITEM;
class BOARD;
class PICKED_ITEMS_LIST;
class PCB_TOOL_BASE;
class TOOL_MANAGER;
class EDA_DRAW_FRAME;
class TOOL_BASE;

#define SKIP_UNDO          0x0001
#define APPEND_UNDO        0x0002
#define SKIP_SET_DIRTY     0x0004
#define SKIP_CONNECTIVITY  0x0008
#define ZONE_FILL_OP       0x0010

class BOARD_COMMIT : public COMMIT
{
public:
    BOARD_COMMIT( TOOL_MANAGER* aToolMgr );
    BOARD_COMMIT( EDA_DRAW_FRAME* aFrame );
    BOARD_COMMIT( PCB_TOOL_BASE *aTool );

    virtual ~BOARD_COMMIT();

    BOARD* GetBoard() const;

    virtual void Push( const wxString& aMessage = wxT( "A commit" ),
                       int aCommitFlags = 0 ) override;

    virtual void Revert() override;
    COMMIT&      Stage( EDA_ITEM* aItem, CHANGE_TYPE aChangeType ) override;
    COMMIT&      Stage( std::vector<EDA_ITEM*>& container, CHANGE_TYPE aChangeType ) override;
    COMMIT&      Stage( const PICKED_ITEMS_LIST& aItems,
                        UNDO_REDO aModFlag = UNDO_REDO::UNSPECIFIED ) override;

private:
    EDA_ITEM* parentObject( EDA_ITEM* aItem ) const override;

    EDA_ITEM* makeImage( EDA_ITEM* aItem ) const override;

    void dirtyIntersectingZones( BOARD_ITEM* item, int aChangeType );

private:
    TOOL_MANAGER*  m_toolMgr;
    bool           m_isFootprintEditor;
    bool           m_isBoardEditor;
};

#endif
