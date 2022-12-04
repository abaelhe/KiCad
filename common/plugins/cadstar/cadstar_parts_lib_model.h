/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2023 Roberto Fernandez Bautista <roberto.fer.bau@gmail.com>
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

#ifndef CADSTAR_PARTS_LIB_MODEL_H
#define CADSTAR_PARTS_LIB_MODEL_H

#include <map>
#include <optional>
#include <string>
#include <vector>

struct CADSTAR_PART_ENTRY;
struct CADSTAR_SWAP_GROUP;
struct CADSTAR_ATTRIBUTE_VALUE;

/**
 * CADSTAR Parts Library (*.lib) model - a data structure describing the contents of the
 * file format
 */
struct CADSTAR_PARTS_LIB_MODEL
{
    std::optional<long>             m_FormatNumber;
    std::vector<CADSTAR_PART_ENTRY> m_PartEntries;
};


struct CADSTAR_PART_ENTRY
{
    std::string                m_Name;
    std::optional<std::string> m_Number;
    std::optional<std::string> m_Version;
    std::optional<std::string> m_Description;
    std::string                m_Pcb_component;
    std::optional<std::string> m_Pcb_alternate;
    std::optional<std::string> m_Value; // *VALUE Note: ? character = the start of a new line
    std::optional<std::string> m_PartDefinitionName; // *DFN
    std::string                m_ComponentStem = "";
    std::optional<long>        m_MaxPinCount;

    std::optional<std::string> m_SpicePartName;
    std::optional<std::string> m_SpiceModel;
    std::optional<std::string> m_SpiceValue;

    std::optional<std::string> m_AcceptancePartName;
    std::optional<std::string> m_AcceptanceText;

    bool m_GateSwappingAllowed = true;
    bool m_PinsVisible = true;

    /**
     * Map of pin numbers to alphanumeric pin names
     * Pin names can be a maximum of 10 characters
     * (Typically used for naming of BGA pads)
     *
     * E.g: *PNM 1=A1 2=A2 3=A3 4=B1 5=B2 6=B3
     */
    std::map<long, std::string> m_PinNamesMap;

    /**
     * Map of pin numbers to alphanumeric pin labels
     *
     * E.g: *PLB 1=STROBE 2=OFFSET 3=OFFSET 5=+ 6=+v
     */
    std::map<long, std::string> m_PinLabelsMap;

    /**
     * Groups of pins that are interchangeable with each other
     *
     * E.g: *EQU 2=1, 6=5, 8=9=10, 12=13
     */
    std::vector<std::vector<long>> m_PinEquivalences;

    /**
     * Groups of INTERNAL gates that are interchangeable with each other
     *
     * E.g: *SYM SYM1
     *      *INT 1 3
     *      *INT 2 5
     *
     * The gate described by pins 1 and 3 above, can be swapped internally with the gate decribed
     * by pins 2 and 5 but they CANNOT be swapped with gates in another part
     */
    std::vector<CADSTAR_SWAP_GROUP> m_InternalSwapGroup;

    /**
     * Groups of EXTERNAL gates that are interchangeable with each other
     *
     * E.g: *SYM SYM2
     *      *EXT 1 3
     *      *EXT 2 5
     *
     * The gate described by pins 1 and 3 above, can be swapped internally with the gate decribed
     * by pins 2 and 5 AND they can be swapped with same gates in another part
     */
    std::vector<CADSTAR_SWAP_GROUP> m_ExternalSwapGroup;

    /**
     * Star (*) line
     * *<User-defined name> <Value>
     * This line is ignored by CADSTAR. Usually they are used by third party tools.
     * These lines are treated as attributes of the Parts library (i.e. Attribute Type = Parts Library).
     */
    std::map<std::string, std::string> m_UserAttributes;

    /**
     * Dollar sign ($) line
     * $[!]<SCM Attribute name>(<Attribute value>)
     * Attributes related to the schematic symbol.
     * Is set to read-only if exclamation mark (!) is present
     */
    std::map<std::string, CADSTAR_ATTRIBUTE_VALUE> m_SchAttributes;

    /**
     * Percentage sign (%) line
     * %[!]<PCB Attribute name>(<Attribute value>)
     * Attributes related to the PCB component / footprint.
     * Is set to read-only if exclamation mark (!) is present
     */
    std::map<std::string, CADSTAR_ATTRIBUTE_VALUE> m_PcbAttributes;

    /**
     * At symbol (@) line
     * [@[!]<SCM/PCB Attribute name>(<Attribute value>)]
     * Attributes related to the PCB component AND the schematic symbol.
     * Is set to read-only if exclamation mark (!) is present
     */
    std::map<std::string, CADSTAR_ATTRIBUTE_VALUE> m_SchAndPcbAttributes;

    /**
     * Tilde (~) line
     * ~[!]<Parts Library Attribute Name>(<Attribute Value>)
     * Attributes related to the Part itself. It cannot be displayed
     * on the PCB or schematic but it is used in CADSTAR to search for
     * parts in the library browser
     * Is set to read-only if exclamation mark (!) is present
     */
    std::map<std::string, CADSTAR_ATTRIBUTE_VALUE> m_PartAttributes;
};


struct CADSTAR_ATTRIBUTE_VALUE
{
    bool        m_ReadOnly = false;
    std::string m_Value;
};


struct CADSTAR_SWAP_GROUP
{
    std::optional<std::string> m_Name;

    /**
     * Each gate is a list of pin numbers. The order of the pins is important
     * as it defines the equivalence between gates
     */
    std::vector<std::vector<long>> m_Gates;
};

#endif //CADSTAR_PARTS_LIB_MODEL_H
