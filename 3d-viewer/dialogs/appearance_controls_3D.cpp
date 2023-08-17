/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2023 CERN
 * Copyright (C) 2023 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <dialogs/appearance_controls_3D.h>

#include <bitmaps.h>
#include <pgm_base.h>
#include <eda_list_dialog.h>
#include <pcb_display_options.h>
#include <eda_3d_viewer_frame.h>
#include <pcbnew_settings.h>
#include <project.h>
#include <settings/settings_manager.h>
#include <settings/color_settings.h>
#include <tool/tool_manager.h>
#include <tools/pcb_actions.h>
#include <widgets/bitmap_toggle.h>
#include <widgets/color_swatch.h>
#include <widgets/grid_bitmap_toggle.h>
#include <dialogs/eda_view_switcher.h>
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>
#include <wx/textdlg.h>


#define RR  APPEARANCE_CONTROLS_3D::APPEARANCE_SETTING_3D   // Render Row abbreviation to reduce source width

/// Template for object appearance settings
const APPEARANCE_CONTROLS_3D::APPEARANCE_SETTING_3D APPEARANCE_CONTROLS_3D::s_layerSettings[] = {

    //     text                              id                           tooltip
    RR( _HKI( "Board Body" ),             LAYER_3D_BOARD,             _HKI( "Show board body" ) ),
    RR( _HKI( "F.Cu" ),                   LAYER_3D_COPPER_TOP,        _HKI( "Show front copper / surface finish color" ) ),
    RR( _HKI( "B.Cu" ),                   LAYER_3D_COPPER_BOTTOM,     _HKI( "Show back copper / surface finish color" ) ),
    RR( _HKI( "Adhesive" ),               LAYER_3D_ADHESIVE,          _HKI( "Show adhesive" ) ),
    RR( _HKI( "Solder Paste" ),           LAYER_3D_SOLDERPASTE,       _HKI( "Show solder paste" ) ),
    RR( _HKI( "F.Silkscreen" ),           LAYER_3D_SILKSCREEN_TOP,    _HKI( "Show front silkscreen" ) ),
    RR( _HKI( "B.Silkscreen" ),           LAYER_3D_SILKSCREEN_BOTTOM, _HKI( "Show back silkscreen" ) ),
    RR( _HKI( "F.Mask" ),                 LAYER_3D_SOLDERMASK_TOP,    _HKI( "Show front solder mask" ) ),
    RR( _HKI( "B.Mask" ),                 LAYER_3D_SOLDERMASK_BOTTOM, _HKI( "Show back solder mask" ) ),
    RR( _HKI( "User.Drawings" ),          LAYER_3D_USER_DRAWINGS,     _HKI( "Show user drawings layer" ) ),
    RR( _HKI( "User.Comments" ),          LAYER_3D_USER_COMMENTS,     _HKI( "Show user comments layer" ) ),
    RR( _HKI( "User.Eco1" ),              LAYER_3D_USER_ECO1,         _HKI( "Show user ECO1 layer" ) ),
    RR( _HKI( "User.Eco2" ),              LAYER_3D_USER_ECO2,         _HKI( "Show user ECO2 layer" ) ),
    RR(),
    RR( _HKI( "Through-hole Models" ),    LAYER_3D_TH_MODELS,         _HKI( "Show models for 'Through hole' footprints" )       ),
    RR( _HKI( "SMD Models" ),             LAYER_3D_SMD_MODELS,        _HKI( "Show models for 'SMD' footprints" )     ),
    RR( _HKI( "Virtual Models" ),         LAYER_3D_VIRTUAL_MODELS,    _HKI( "Show models for 'Unspecified' footprints" )     ),
    RR( _HKI( "Models not in POS File" ), LAYER_3D_MODELS_NOT_IN_POS, _HKI( "Show models even if not in POS file" ) ),
    RR( _HKI( "Models marked DNP" ),      LAYER_3D_MODELS_MARKED_DNP, _HKI( "Show models even if marked as DNP" )  ),
    RR( _HKI( "Model Bounding Boxes" ),   LAYER_3D_BOUNDING_BOXES,    _HKI( "Show model bounding boxes in realtime renderer" ) ),
    RR(),
    RR( _HKI( "Values" ),                 LAYER_FP_VALUES,            _HKI( "Show footprint values" ) ),
    RR( _HKI( "References" ),             LAYER_FP_REFERENCES,        _HKI( "Show footprint references" ) ),
    RR( _HKI( "Footprint Text" ),         LAYER_FP_TEXT,              _HKI( "Show all footprint text" ) ),
    RR(),
    RR( _HKI( "3D Axis" ),                LAYER_3D_AXES,              _HKI( "Show 3D axes indicator" ) ),
    RR( _HKI( "Background Start" ),       LAYER_3D_BACKGROUND_TOP,    _HKI( "Background gradient start color" ) ),
    RR( _HKI( "Background End" ),         LAYER_3D_BACKGROUND_BOTTOM, _HKI( "Background gradient end color" ) ),
};


APPEARANCE_CONTROLS_3D::APPEARANCE_CONTROLS_3D( EDA_3D_VIEWER_FRAME* aParent,
                                                wxWindow* aFocusOwner ) :
        APPEARANCE_CONTROLS_3D_BASE( aParent ),
        m_frame( aParent ),
        m_focusOwner( aFocusOwner ),
        m_lastSelectedViewport( nullptr )
{
    DPI_SCALING dpi( nullptr, m_frame );

    int indicatorSize = ConvertDialogToPixels( wxSize( 6, 6 ) ).x / dpi.GetContentScaleFactor();
    int screenHeight  = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );
    m_pointSize       = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT ).GetPointSize();

    m_layerPanelColour = m_panelLayers->GetBackgroundColour().ChangeLightness( 110 );
    SetBorders( true, false, false, false );

    m_layersOuterSizer = new wxBoxSizer( wxVERTICAL );
    m_windowLayers->SetSizer( m_layersOuterSizer );
    m_windowLayers->SetScrollRate( 0, 5 );
    m_windowLayers->Bind( wxEVT_SET_FOCUS, &APPEARANCE_CONTROLS_3D::OnSetFocus, this );

    m_envOuterSizer = new wxBoxSizer( wxVERTICAL );

    wxFont infoFont = KIUI::GetInfoFont( this );
    m_panelLayers->SetFont( infoFont );
    m_windowLayers->SetFont( infoFont );
    m_presetsLabel->SetFont( infoFont );
    m_viewportsLabel->SetFont( infoFont );

    m_cbLayerPresets->SetToolTip( wxString::Format( _( "Save and restore color and visibility "
                                                       "combinations.\n"
                                                       "Use %s+Tab to activate selector.\n"
                                                       "Successive Tabs while holding %s down will "
                                                       "cycle through presets in the popup." ),
                                                    KeyNameFromKeyCode( PRESET_SWITCH_KEY ),
                                                    KeyNameFromKeyCode( PRESET_SWITCH_KEY ) ) );

    m_cbViewports->SetToolTip( wxString::Format( _( "Save and restore camera position and zoom.\n"
                                                    "Use %s+Tab to activate selector.\n"
                                                    "Successive Tabs while holding %s down will "
                                                    "cycle through viewports in the popup." ),
                                                 KeyNameFromKeyCode( VIEWPORT_SWITCH_KEY ),
                                                 KeyNameFromKeyCode( VIEWPORT_SWITCH_KEY ) ) );

    if( screenHeight <= 900 && m_pointSize >= indicatorSize )
        m_pointSize = m_pointSize * 8 / 10;

    m_cbLayerPresets->Bind( wxEVT_CHOICE, &APPEARANCE_CONTROLS_3D::onLayerPresetChanged, this );

    m_toggleGridRenderer = new GRID_BITMAP_TOGGLE_RENDERER( KiBitmap( BITMAPS::visibility ),
                                                            KiBitmap( BITMAPS::visibility_off ) );
}


APPEARANCE_CONTROLS_3D::~APPEARANCE_CONTROLS_3D()
{
}


wxSize APPEARANCE_CONTROLS_3D::GetBestSize() const
{
    DPI_SCALING dpi( nullptr, m_frame );
    wxSize      size( 220 * dpi.GetScaleFactor(), 480 * dpi.GetScaleFactor() );
    return size;
}


void APPEARANCE_CONTROLS_3D::OnSetFocus( wxFocusEvent& aEvent )
{
#ifdef __WXMSW__
    // In wxMSW, buttons won't process events unless they have focus, so we'll let it take the
    // focus and give it back to the parent in the button event handler.
    if( wxBitmapButton* btn = dynamic_cast<wxBitmapButton*>( aEvent.GetEventObject() ) )
    {
        wxCommandEvent evt( wxEVT_BUTTON );
        wxPostEvent( btn, evt );
    }
#endif

    passOnFocus();
    aEvent.Skip();
}


void APPEARANCE_CONTROLS_3D::OnSize( wxSizeEvent& aEvent )
{
    aEvent.Skip();
}


void APPEARANCE_CONTROLS_3D::OnLanguageChanged()
{
    Freeze();
    rebuildLayers();
    rebuildLayerPresetsWidget();
    rebuildViewportsWidget();

    Thaw();
    Refresh();
}


void APPEARANCE_CONTROLS_3D::OnDarkModeToggle()
{
    // This is essentially a list of hacks because DarkMode isn't yet implemented inside
    // wxWidgets.
    //
    // The individual wxPanels, COLOR_SWATCHes and GRID_CELL_COLOR_RENDERERs should really be
    // overriding some virtual method or responding to some wxWidgets event so that the parent
    // doesn't have to know what it contains.  But, that's not where we are, so... :shrug:

    m_layerPanelColour = m_panelLayers->GetBackgroundColour().ChangeLightness( 110 );

    m_windowLayers->SetBackgroundColour( m_layerPanelColour );

    for( wxSizerItem* child : m_layersOuterSizer->GetChildren() )
    {
        if( child && child->GetWindow() )
            child->GetWindow()->SetBackgroundColour( m_layerPanelColour );
    }
}


void APPEARANCE_CONTROLS_3D::CommonSettingsChanged()
{
    OnLanguageChanged();
}


void APPEARANCE_CONTROLS_3D::ApplyLayerPreset( const wxString& aPresetName )
{
    if( aPresetName == FOLLOW_PCB || aPresetName == FOLLOW_PLOT_SETTINGS )
    {
        m_frame->GetAdapter().m_Cfg->m_CurrentPreset = aPresetName;
        UpdateLayerCtls();
        m_frame->NewDisplay( true );
    }
    else if( LAYER_PRESET_3D* preset = m_frame->GetAdapter().m_Cfg->FindPreset( aPresetName ) )
    {
        doApplyLayerPreset( *preset );
    }

    // Move to front of MRU list
    if( m_presetMRU.Index( aPresetName ) != wxNOT_FOUND )
        m_presetMRU.Remove( aPresetName );

    m_presetMRU.Insert( aPresetName, 0 );

    updateLayerPresetWidget( aPresetName );
}


std::vector<VIEWPORT3D> APPEARANCE_CONTROLS_3D::GetUserViewports() const
{
    std::vector<VIEWPORT3D> ret;

    for( const auto& [name, viewport] : m_viewports )
        ret.emplace_back( viewport );

    return ret;
}


void APPEARANCE_CONTROLS_3D::SetUserViewports( std::vector<VIEWPORT3D>& aViewportList )
{
    m_viewports.clear();

    for( const VIEWPORT3D& viewport : aViewportList )
    {
        if( m_viewports.count( viewport.name ) )
            continue;

        m_viewports[viewport.name] = viewport;

        m_viewportMRU.Add( viewport.name );
    }

    rebuildViewportsWidget();

    // Now is as good a time as any to initialize the layer presets as well.
    rebuildLayerPresetsWidget();

    m_presetMRU.Add( FOLLOW_PCB );
    m_presetMRU.Add( FOLLOW_PLOT_SETTINGS );

    for( const LAYER_PRESET_3D& preset : m_frame->GetAdapter().m_Cfg->m_LayerPresets )
        m_presetMRU.Add( preset.name );
}


void APPEARANCE_CONTROLS_3D::ApplyViewport( const wxString& aViewportName )
{
    int idx = m_cbViewports->FindString( aViewportName );

    if( idx >= 0 && idx < (int)m_cbViewports->GetCount() - 3 /* separator */ )
    {
        m_cbViewports->SetSelection( idx );
        m_lastSelectedViewport = static_cast<VIEWPORT3D*>( m_cbViewports->GetClientData( idx ) );
    }
    else
    {
        m_cbViewports->SetSelection( m_cbViewports->GetCount() - 3 ); // separator
        m_lastSelectedViewport = nullptr;
    }

    if( m_lastSelectedViewport )
        doApplyViewport( *m_lastSelectedViewport );
}


void APPEARANCE_CONTROLS_3D::OnLayerVisibilityChanged( int aLayer, bool isVisible )
{
    std::bitset<LAYER_3D_END>     visibleLayers = m_frame->GetAdapter().GetVisibleLayers();
    const std::map<int, COLOR4D>& colors = m_frame->GetAdapter().GetLayerColors();
    bool                          killFollow = false;

    // Special-case controls
    switch( aLayer )
    {
    case LAYER_FP_TEXT:
        // Because Footprint Text is a meta-control that also can disable values/references,
        // drag them along here so that the user is less likely to be confused.
        visibleLayers.set( LAYER_FP_REFERENCES, isVisible );
        visibleLayers.set( LAYER_FP_VALUES, isVisible );
        visibleLayers.set( LAYER_FP_TEXT, isVisible );
        killFollow = true;
        break;

    case LAYER_FP_REFERENCES:
    case LAYER_FP_VALUES:
        // In case that user changes Footprint Value/References when the Footprint Text
        // meta-control is disabled, we should put it back on.
        if( isVisible )
            OnLayerVisibilityChanged( LAYER_FP_TEXT, isVisible );

        visibleLayers.set( aLayer, isVisible );
        killFollow = true;
        break;

    case LAYER_3D_COPPER_TOP:
    case LAYER_3D_COPPER_BOTTOM:
    case LAYER_3D_SILKSCREEN_BOTTOM:
    case LAYER_3D_SILKSCREEN_TOP:
    case LAYER_3D_SOLDERMASK_BOTTOM:
    case LAYER_3D_SOLDERMASK_TOP:
    case LAYER_3D_SOLDERPASTE:
    case LAYER_3D_ADHESIVE:
    case LAYER_3D_USER_COMMENTS:
    case LAYER_3D_USER_DRAWINGS:
    case LAYER_3D_USER_ECO1:
    case LAYER_3D_USER_ECO2:
        visibleLayers.set( aLayer, isVisible );
        killFollow = true;
        break;

    default:
        visibleLayers.set( aLayer, isVisible );
        break;
    }

    m_frame->GetAdapter().SetVisibleLayers( visibleLayers );
    m_frame->GetAdapter().SetLayerColors( colors );

    const wxString& currentPreset = m_frame->GetAdapter().m_Cfg->m_CurrentPreset;

    if( ( currentPreset != FOLLOW_PCB && currentPreset != FOLLOW_PLOT_SETTINGS ) || killFollow )
        syncLayerPresetSelection();

    UpdateLayerCtls();
    m_frame->NewDisplay( true );
}


void APPEARANCE_CONTROLS_3D::onColorSwatchChanged( COLOR_SWATCH* aSwatch )
{
    std::bitset<LAYER_3D_END> visibleLayers = m_frame->GetAdapter().GetVisibleLayers();
    std::map<int, COLOR4D>    colors = m_frame->GetAdapter().GetLayerColors();

    m_frame->GetAdapter().SetVisibleLayers( visibleLayers );
    m_frame->GetAdapter().SetLayerColors( colors );

    int     layer = aSwatch->GetId();
    COLOR4D newColor = aSwatch->GetSwatchColor();

    colors[ layer ] = newColor;

    if( layer == LAYER_3D_COPPER_TOP )
        colors[ LAYER_3D_COPPER_BOTTOM ] = newColor;
    else if( layer == LAYER_3D_COPPER_BOTTOM )
        colors[ LAYER_3D_COPPER_TOP ] = newColor;

    m_frame->GetAdapter().SetLayerColors( colors );

    syncLayerPresetSelection();

    m_frame->NewDisplay( true );
}


void APPEARANCE_CONTROLS_3D::rebuildLayers()
{
    int swatchWidth = m_windowLayers->ConvertDialogToPixels( wxSize( 8, 0 ) ).x;

    std::bitset<LAYER_3D_END> visibleLayers = m_frame->GetAdapter().GetVisibleLayers();
    std::map<int, COLOR4D>    colors = m_frame->GetAdapter().GetLayerColors();
    std::map<int, COLOR4D>    defaultColors = m_frame->GetAdapter().GetDefaultColors();

    m_layerSettings.clear();
    m_layersOuterSizer->Clear( true );
    m_layersOuterSizer->AddSpacer( 5 );

    m_envOuterSizer->Clear( true );

    auto appendLayer =
            [&]( const std::unique_ptr<APPEARANCE_SETTING_3D>& aSetting )
            {
                wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL );
                int         layer = aSetting->id;

                aSetting->visible = visibleLayers.test( layer );

                if( colors.count( layer ) )
                {
                    COLOR_SWATCH* swatch = new COLOR_SWATCH( m_windowLayers, colors[ layer ], layer,
                                                             COLOR4D::WHITE, defaultColors[ layer ],
                                                             SWATCH_SMALL );
                    swatch->SetToolTip( _( "Left double click or middle click for color change, "
                                           "right click for menu" ) );

                    sizer->Add( swatch, 0,  wxALIGN_CENTER_VERTICAL, 0 );
                    aSetting->ctl_color = swatch;

                    swatch->Bind( COLOR_SWATCH_CHANGED,
                            [this]( wxCommandEvent& event )
                            {
                                auto swatch = static_cast<COLOR_SWATCH*>( event.GetEventObject() );
                                onColorSwatchChanged( swatch );

                                passOnFocus();
                            } );
                }
                else
                {
                    sizer->AddSpacer( swatchWidth  );
                }

                sizer->AddSpacer( 5 );

                wxStaticText* label = new wxStaticText( m_windowLayers, layer, aSetting->label );
                label->Wrap( -1 );
                label->SetToolTip( aSetting->tooltip );

                if( layer == LAYER_3D_BACKGROUND_TOP || layer == LAYER_3D_BACKGROUND_BOTTOM )
                {
                    sizer->AddSpacer( swatchWidth );
                }
                else
                {
                    BITMAP_TOGGLE* btn_visible = new BITMAP_TOGGLE( m_windowLayers, layer,
                                                                    KiBitmap( BITMAPS::visibility ),
                                                                    KiBitmap( BITMAPS::visibility_off ),
                                                                    aSetting->visible );

                    btn_visible->Bind( TOGGLE_CHANGED,
                            [this]( wxCommandEvent& aEvent )
                            {
                                int id = static_cast<wxWindow*>( aEvent.GetEventObject() )->GetId();
                                bool isVisible = aEvent.GetInt();
                                OnLayerVisibilityChanged( id, isVisible );

                                passOnFocus();
                            } );

                    wxString tip;
                    tip.Printf( _( "Show or hide %s" ), aSetting->label.Lower() );
                    btn_visible->SetToolTip( tip );

                    aSetting->ctl_visibility = btn_visible;
                    sizer->Add( btn_visible, 0, wxALIGN_CENTER_VERTICAL, 0 );
                }

                sizer->AddSpacer( 5 );
                sizer->Add( label, 0, wxALIGN_CENTER_VERTICAL, 0 );

                m_layersOuterSizer->Add( sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 5 );
                m_layersOuterSizer->AddSpacer( 2 );
            };

    for( const APPEARANCE_SETTING_3D& s_setting : s_layerSettings )
    {
        m_layerSettings.emplace_back( std::make_unique<APPEARANCE_SETTING_3D>( s_setting ) );
        std::unique_ptr<APPEARANCE_SETTING_3D>& setting = m_layerSettings.back();

        // Because s_render_rows is created static, we must explicitly call wxGetTranslation
        // for texts which are internationalized (tool tips and item names)
        setting->tooltip = wxGetTranslation( s_setting.tooltip );
        setting->label   = wxGetTranslation( s_setting.label );

        if( setting->spacer )
            m_layersOuterSizer->AddSpacer( m_pointSize );
        else
            appendLayer( setting );

        m_layerSettingsMap[setting->id] = setting.get();
    }

    m_sizerOuter->Layout();
}


void APPEARANCE_CONTROLS_3D::UpdateLayerCtls()
{
    std::bitset<LAYER_3D_END> visibleLayers = m_frame->GetAdapter().GetVisibleLayers();
    std::map<int, COLOR4D>    colors = m_frame->GetAdapter().GetLayerColors();

    for( std::unique_ptr<APPEARANCE_SETTING_3D>& setting : m_layerSettings )
    {
        if( setting->spacer )
            continue;

        if( setting->ctl_visibility )
            setting->ctl_visibility->SetValue( visibleLayers.test( setting->id ) );

        if( setting->ctl_color )
            setting->ctl_color->SetSwatchColor( colors[ setting->id ], false );
    }
}


void APPEARANCE_CONTROLS_3D::rebuildLayerPresetsWidget()
{
    m_viewportsLabel->SetLabel( wxString::Format( _( "Presets (%s+Tab):" ),
                                                  KeyNameFromKeyCode( PRESET_SWITCH_KEY ) ) );

    m_cbLayerPresets->Clear();

    // Build the layers preset list.

    m_cbLayerPresets->Append( _( "Follow PCB Editor" ) );
    m_cbLayerPresets->Append( _( "Follow PCB Plot Settings" ) );

    for( const LAYER_PRESET_3D& preset : m_frame->GetAdapter().m_Cfg->m_LayerPresets )
        m_cbLayerPresets->Append( preset.name );

    m_cbLayerPresets->Append( wxT( "---" ) );
    m_cbLayerPresets->Append( _( "Save preset..." ) );
    m_cbLayerPresets->Append( _( "Delete preset..." ) );

    updateLayerPresetWidget( m_frame->GetAdapter().m_Cfg->m_CurrentPreset );
}


void APPEARANCE_CONTROLS_3D::syncLayerPresetSelection()
{
    m_frame->GetAdapter().m_Cfg->m_CurrentPreset = wxEmptyString;

    std::vector<LAYER_PRESET_3D>& presets = m_frame->GetAdapter().m_Cfg->m_LayerPresets;
    std::bitset<LAYER_3D_END>     visibleLayers = m_frame->GetAdapter().GetVisibleLayers();
    std::map<int, COLOR4D>        colors = m_frame->GetAdapter().GetLayerColors();

    auto it = std::find_if( presets.begin(), presets.end(),
            [&]( const LAYER_PRESET_3D& aPreset )
            {
                for( int layer = LAYER_3D_BOARD; layer < LAYER_3D_END; ++layer )
                {
                    if( aPreset.layers.test( layer ) != visibleLayers.test( layer ) )
                        return false;
                }

                for( int layer : { LAYER_FP_REFERENCES, LAYER_FP_VALUES, LAYER_FP_TEXT } )
                {
                    if( aPreset.layers.test( layer ) != visibleLayers.test( layer ) )
                        return false;
                }

                for( int layer = LAYER_3D_BOARD; layer < LAYER_3D_ADHESIVE; ++layer )
                {
                    if( aPreset.colors.at( layer ) != colors.at( layer ) )
                        return false;
                }

                return true;
            } );

    if( it != presets.end() )
    {
        m_frame->GetAdapter().m_Cfg->m_CurrentPreset = it->name;
        m_cbLayerPresets->SetStringSelection( it->name );
    }
    else
    {
        m_cbLayerPresets->SetSelection( m_cbLayerPresets->GetCount() - 3 ); // separator
    }
}


void APPEARANCE_CONTROLS_3D::updateLayerPresetWidget( const wxString& aName )
{
    if( aName == FOLLOW_PCB )
        m_cbLayerPresets->SetSelection( 0 );
    else if( aName == FOLLOW_PLOT_SETTINGS )
        m_cbLayerPresets->SetSelection( 1 );
    else if( !m_cbLayerPresets->SetStringSelection( aName ) )
        m_cbLayerPresets->SetSelection( m_cbLayerPresets->GetCount() - 3 ); // separator
}


void APPEARANCE_CONTROLS_3D::onLayerPresetChanged( wxCommandEvent& aEvent )
{
    EDA_3D_VIEWER_SETTINGS*   cfg = m_frame->GetAdapter().m_Cfg;
    int                       count = m_cbLayerPresets->GetCount();
    int                       index = m_cbLayerPresets->GetSelection();
    wxString                  name;

    if( index == 0 )
    {
        name = FOLLOW_PCB;
        cfg->m_CurrentPreset = name;
        UpdateLayerCtls();
        m_frame->NewDisplay( true );
    }
    else if( index == 1 )
    {
        name = FOLLOW_PLOT_SETTINGS;
        cfg->m_CurrentPreset = name;
        UpdateLayerCtls();
        m_frame->NewDisplay( true );
    }
    else if( index == count - 3 )
    {
        // Separator: reject the selection
        updateLayerPresetWidget( cfg->m_CurrentPreset );
        return;
    }
    else if( index == count - 2 )
    {
        wxTextEntryDialog dlg( this, _( "Layer preset name:" ), _( "Save Layer Preset" ) );

        if( dlg.ShowModal() != wxID_OK )
        {
            updateLayerPresetWidget( cfg->m_CurrentPreset );
            return;
        }
        else
        {
            std::bitset<LAYER_3D_END> visibleLayers = m_frame->GetAdapter().GetVisibleLayers();
            std::map<int, COLOR4D>    colors = m_frame->GetAdapter().GetLayerColors();

            name = dlg.GetValue();

            if( LAYER_PRESET_3D* preset = cfg->FindPreset( name ) )
            {
                preset->layers = visibleLayers;
                preset->colors = colors;
                m_cbLayerPresets->SetSelection( m_cbLayerPresets->FindString( name ) );
            }
            else
            {
                cfg->m_LayerPresets.emplace_back( name, visibleLayers, colors );
                m_cbLayerPresets->SetSelection( m_cbLayerPresets->Insert( name, index - 1 ) );
            }

            cfg->m_CurrentPreset = name;
        }
    }
    else if( index == count - 1 )
    {
        wxArrayString              headers;
        std::vector<wxArrayString> items;

        headers.Add( _( "Presets" ) );

        for( LAYER_PRESET_3D& preset : cfg->m_LayerPresets )
        {
            wxArrayString item;
            item.Add( preset.name );
            items.emplace_back( item );
        }

        EDA_LIST_DIALOG dlg( m_frame, _( "Delete Preset" ), headers, items );
        dlg.SetListLabel( _( "Select preset:" ) );

        if( dlg.ShowModal() == wxID_OK )
        {
            name = dlg.GetTextSelection();

            if( m_cbLayerPresets->FindString( name ) != wxNOT_FOUND )
                m_cbLayerPresets->Delete( m_cbLayerPresets->FindString( name ) );

            alg::delete_if( cfg->m_LayerPresets,
                    [name]( const LAYER_PRESET_3D& preset )
                    {
                        return preset.name == name;
                    } );

            if( cfg->m_CurrentPreset == name )
                cfg->m_CurrentPreset = wxEmptyString;

            m_presetMRU.Remove( name );
        }

        updateLayerPresetWidget( cfg->m_CurrentPreset );
        return;
    }
    else if( LAYER_PRESET_3D* preset = cfg->FindPreset( m_cbLayerPresets->GetStringSelection() ) )
    {
        name = preset->name;
        doApplyLayerPreset( *preset );
    }

    // Move to front of MRU list
    if( m_presetMRU.Index( name ) != wxNOT_FOUND )
        m_presetMRU.Remove( name );

    m_presetMRU.Insert( name, 0 );

    passOnFocus();
}


void APPEARANCE_CONTROLS_3D::doApplyLayerPreset( const LAYER_PRESET_3D& aPreset )
{
    BOARD_ADAPTER& adapter = m_frame->GetAdapter();

    adapter.m_Cfg->m_CurrentPreset = aPreset.name;
    adapter.SetVisibleLayers( aPreset.layers );
    adapter.SetLayerColors( aPreset.colors );

    UpdateLayerCtls();
    m_frame->NewDisplay( true );
}


void APPEARANCE_CONTROLS_3D::rebuildViewportsWidget()
{
    m_viewportsLabel->SetLabel( wxString::Format( _( "Viewports (%s+Tab):" ),
                                                  KeyNameFromKeyCode( VIEWPORT_SWITCH_KEY ) ) );

    m_cbViewports->Clear();

    for( std::pair<const wxString, VIEWPORT3D>& pair : m_viewports )
        m_cbViewports->Append( pair.first, static_cast<void*>( &pair.second ) );

    m_cbViewports->Append( wxT( "---" ) );
    m_cbViewports->Append( _( "Save viewport..." ) );
    m_cbViewports->Append( _( "Delete viewport..." ) );

    m_cbViewports->SetSelection( m_cbViewports->GetCount() - 3 );
    m_lastSelectedViewport = nullptr;
}


void APPEARANCE_CONTROLS_3D::onViewportChanged( wxCommandEvent& aEvent )
{
    int count = m_cbViewports->GetCount();
    int index = m_cbViewports->GetSelection();

    if( index >= 0 && index < count - 3 )
    {
        VIEWPORT3D* viewport = static_cast<VIEWPORT3D*>( m_cbViewports->GetClientData( index ) );

        wxCHECK( viewport, /* void */ );

        doApplyViewport( *viewport );

        if( !viewport->name.IsEmpty() )
        {
            m_viewportMRU.Remove( viewport->name );
            m_viewportMRU.Insert( viewport->name, 0 );
        }
    }
    else if( index == count - 2 )
    {
        // Save current state to new preset
        wxString name;

        wxTextEntryDialog dlg( this, _( "Viewport name:" ), _( "Save Viewport" ), name );

        if( dlg.ShowModal() != wxID_OK )
        {
            if( m_lastSelectedViewport )
                m_cbViewports->SetStringSelection( m_lastSelectedViewport->name );
            else
                m_cbViewports->SetSelection( m_cbViewports->GetCount() - 3 );

            return;
        }

        name = dlg.GetValue();
        bool exists = m_viewports.count( name );

        if( !exists )
        {
            m_viewports[name] = VIEWPORT3D( name, m_frame->GetCurrentCamera().GetViewMatrix() );

            index = m_cbViewports->Insert( name, index-1, static_cast<void*>( &m_viewports[name] ) );
        }
        else
        {
            m_viewports[name].matrix = m_frame->GetCurrentCamera().GetViewMatrix();
            index = m_cbViewports->FindString( name );
            m_viewportMRU.Remove( name );
        }

        m_cbViewports->SetSelection( index );
        m_viewportMRU.Insert( name, 0 );

        return;
    }
    else if( index == count - 1 )
    {
        // Delete an existing viewport
        wxArrayString headers;
        std::vector<wxArrayString> items;

        headers.Add( _( "Viewports" ) );

        for( std::pair<const wxString, VIEWPORT3D>& pair : m_viewports )
        {
            wxArrayString item;
            item.Add( pair.first );
            items.emplace_back( item );
        }

        EDA_LIST_DIALOG dlg( m_frame, _( "Delete Viewport" ), headers, items );
        dlg.SetListLabel( _( "Select viewport:" ) );

        if( dlg.ShowModal() == wxID_OK )
        {
            wxString viewportName = dlg.GetTextSelection();
            int idx = m_cbViewports->FindString( viewportName );

            if( idx != wxNOT_FOUND )
            {
                m_viewports.erase( viewportName );
                m_cbViewports->Delete( idx );
                m_viewportMRU.Remove( viewportName );
            }
        }

        if( m_lastSelectedViewport )
            m_cbViewports->SetStringSelection( m_lastSelectedViewport->name );
        else
            m_cbViewports->SetSelection( m_cbViewports->GetCount() - 3 );

        return;
    }

    passOnFocus();
}


void APPEARANCE_CONTROLS_3D::onUpdateViewportsCb( wxUpdateUIEvent& aEvent )
{
    int count = m_cbViewports->GetCount();
    int index = m_cbViewports->GetSelection();

    if( index >= 0 && index < count - 3 )
    {
        VIEWPORT3D* viewport = static_cast<VIEWPORT3D*>( m_cbViewports->GetClientData( index ) );

        wxCHECK( viewport, /* void */ );

        if( m_frame->GetCurrentCamera().GetViewMatrix() != viewport->matrix )
            m_cbViewports->SetSelection( -1 );
    }
}


void APPEARANCE_CONTROLS_3D::doApplyViewport( const VIEWPORT3D& aViewport )
{
    m_frame->GetCurrentCamera().SetViewMatrix( aViewport.matrix );

    if( m_frame->GetAdapter().m_Cfg->m_Render.engine == RENDER_ENGINE::OPENGL )
        m_frame->GetCanvas()->Request_refresh();
    else
        m_frame->GetCanvas()->RenderRaytracingRequest();
}


void APPEARANCE_CONTROLS_3D::passOnFocus()
{
    m_focusOwner->SetFocus();
}


