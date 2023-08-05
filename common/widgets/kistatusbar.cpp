/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2023 Mark Roszko <mark.roszko@gmail.com>
 * Copyright (C) 2023 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <wx/button.h>
#include <wx/statusbr.h>
#include <wx/gauge.h>
#include <wx/stattext.h>
#include <array>
#include <widgets/kistatusbar.h>
#include <widgets/bitmap_button.h>
#include <pgm_base.h>
#include <background_jobs_monitor.h>
#include <notifications_manager.h>
#include <bitmaps.h>

KISTATUSBAR::KISTATUSBAR( int aNumberFields, wxWindow* parent, wxWindowID id ) :
        wxStatusBar( parent, id ),
        m_normalFieldsCount( aNumberFields )
{
    const int ExtraFields = 4;
    SetFieldsCount( aNumberFields + ExtraFields );

    int* widths = new int[aNumberFields + ExtraFields];
    for( int i = 0; i < aNumberFields; i++ )
        widths[i] = -1;

    widths[aNumberFields] = 200;
    widths[aNumberFields + 1] = 75;
    widths[aNumberFields + 2] = 20;
    widths[aNumberFields + 3] = 20;

    SetStatusWidths( aNumberFields + ExtraFields, widths );
    delete[] widths;


    int* styles = new int[aNumberFields + ExtraFields];
    for( int i = 0; i < aNumberFields + ExtraFields; i++ )
        styles[i] = wxSB_FLAT;

    SetStatusStyles( aNumberFields + ExtraFields, styles );
    delete[] styles;

    m_backgroundTxt =
            new wxStaticText( this, wxID_ANY, wxT( "" ), wxDefaultPosition, wxDefaultSize );


    m_backgroundProgressBar = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize,
                                           wxGA_HORIZONTAL | wxGA_SMOOTH );

    m_backgroundStopButton =
            new wxButton( this, wxID_ANY, "X", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );

    m_notificationsButton = new BITMAP_BUTTON( this, wxID_ANY, wxNullBitmap, wxDefaultPosition,
                                               wxDefaultSize, wxBU_EXACTFIT );

    m_notificationsButton->SetPadding( 0 );
    m_notificationsButton->SetBitmap( KiBitmap( BITMAPS::notifications ) );
    m_notificationsButton->SetShowBadge( true );
    m_notificationsButton->SetBitmapCentered( true );

    m_notificationsButton->Bind( wxEVT_BUTTON, &KISTATUSBAR::onNotificationsIconClick, this );

    Bind( wxEVT_SIZE, &KISTATUSBAR::onSize, this );
    m_backgroundProgressBar->Bind( wxEVT_LEFT_DOWN, &KISTATUSBAR::onBackgroundProgressClick, this );

    HideBackgroundProgressBar();
    Layout();
}


void KISTATUSBAR::onNotificationsIconClick( wxCommandEvent& aEvent )
{
    wxPoint pos = m_notificationsButton->GetScreenPosition();

    wxRect r;
    GetFieldRect( m_normalFieldsCount + 3, r );
    pos.x += r.GetWidth();

    Pgm().GetNotificationsManager().ShowList( this, pos );
}


void KISTATUSBAR::onBackgroundProgressClick( wxMouseEvent& aEvent )
{
    wxPoint pos = m_backgroundProgressBar->GetScreenPosition();

    wxRect r;
    GetFieldRect( m_normalFieldsCount + 1, r );
    pos.x += r.GetWidth();

    Pgm().GetBackgroundJobMonitor().ShowList( this, pos );
}


void KISTATUSBAR::onSize( wxSizeEvent& aEvent )
{
    wxRect r;
    GetFieldRect( m_normalFieldsCount, r );
    int x = r.GetLeft();
    int y = r.GetTop();

    m_backgroundTxt->SetPosition( { x, y } );

    GetFieldRect( m_normalFieldsCount + 1, r );
    x = r.GetLeft();
    y = r.GetTop();
    int           w = r.GetWidth();
    int           h = r.GetHeight();
    constexpr int b = 5;

    auto buttonSize = m_backgroundStopButton->GetEffectiveMinSize();
    m_backgroundStopButton->SetPosition( { x + w - buttonSize.GetWidth(), y } );
    m_backgroundStopButton->SetSize( buttonSize.GetWidth(), h );

    m_backgroundProgressBar->SetPosition( { x, y } );
    m_backgroundProgressBar->SetSize( w - buttonSize.GetWidth() - b, h );

    GetFieldRect( m_normalFieldsCount+3, r );
    x = r.GetLeft();
    y = r.GetTop();
    h = r.GetHeight();
    buttonSize = m_notificationsButton->GetEffectiveMinSize();
    m_notificationsButton->SetPosition( { x, y } );
    m_notificationsButton->SetSize( buttonSize.GetWidth() + 6, h );
}


void KISTATUSBAR::ShowBackgroundProgressBar( bool aCancellable )
{
    m_backgroundProgressBar->Show();

    if( aCancellable )
        m_backgroundStopButton->Show();
    else
        m_backgroundStopButton->Hide();
}


void KISTATUSBAR::HideBackgroundProgressBar()
{
    m_backgroundProgressBar->Hide();
    m_backgroundStopButton->Hide();
}


void KISTATUSBAR::SetBackgroundProgress( int aAmount )
{
    m_backgroundProgressBar->SetValue( aAmount );
}


void KISTATUSBAR::SetBackgroundProgressMax( int aAmount )
{
    m_backgroundProgressBar->SetRange( aAmount );
}


void KISTATUSBAR::SetBackgroundStatusText( const wxString& aTxt )
{
    m_backgroundTxt->SetLabel( aTxt );
}


void KISTATUSBAR::SetNotificationCount(int aCount)
{
    wxString cnt = "";
    if( aCount > 0 )
    {
        cnt = wxString::Format( "%d", aCount );
    }

    m_notificationsButton->SetBadgeText( cnt );

    // force a repaint or it wont until it gets activity
    Refresh();
}