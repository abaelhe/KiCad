#!/bin/bash

# This script is disgusting (very slow and hacky). Please rewrite it in Python if you have time.


MODELS=$(cat << END
    hicum2    NPN  PNP  Q 3 8  0
    jfet2     NJF  PJF  J 3 2  0
    mesa      NMF  PMF  Z 3 2  0
    hfet1     NMF  PMF  Z 4 0  0
    hfet2     NMF  PMF  Z 3 0  0
    hisimhv1  NMOS PMOS M 4 73 1.2.4
    hisimhv2  NMOS PMOS M 4 73 2.2.0
END
)


UNITS=$(cat << END
    %/deg C
        exponent alternative

    -
        exponent
        normalized length

    ohm/m
        sheet resistance
        resistance per meter
        resistance per metre
        resistance per length
        resistance per unit length

    /ohm m
        conductance per meter
        conductance per metre

    ohm
        resistance
        impedance
        resistor model default value

    F/m^2
        cap per area
    
    F/m
        capacitance per meter
        capacitance per metre
        overlap cap
        capacitance per length
        capacitance per unit length
        capacitance grading coefficient per unit length

    F
        capacitance
        cap\.
    
    H/m
        inductance per meter
        inductance per metre
        inductance per length
        inductance per unit length
    
    H
        inductance

    1/W
        coef of thermal current reduction

    sqrt V
        bulk effect coefficient 1
        bulk threshold parameter

    1/V
        channel length modulation
        vgs dependence on mobility

    V/cm
        Crit\. field for mob\. degradation

    V
        voltage
        potential
    
    A/V^2
        transconductance parameter

    A/m^2
        current density

    A/m
        current per meter
        current per metre
        current per length
        current per unit length

    A
        current

    ohm/deg C^2
        second order temp\. coefficient

    ohm/deg C
        first order temp\. coefficient

    1/deg C^2
        grading coefficient 1st temp\. coeff

    1/deg C
        grading coefficient 2nd temp\. coeff

    deg C/W
        thermal resistance

    deg C
        temperature

    eV
        energy
    
    cm^2/V^2 s
        VBS dependence on muz
        VBS dependence on mus
        VDS dependence on mus

    cm^2/V s
        zero field mobility
        surface mobility

    um/V^2
        VDS depence of u1
    
    um
        .* in um
    
    1/cm^3
        substrate doping

    1/cm^2
        surface state density
        fast surface state density

    m/s
        velocity

    m
        length
        width
        thickness
        narrowing of
        shortening of

    C
        epi charge parameter

    s
        time
        delay

    Hz
        frequency

    deg
        excess phase

    -
        .*
END
)

category()
{
    model_name="$1"
    param_name="$2"
    param_flags="$3"
    param_description="$4"
    is_instance_param="$5"

    if [ "$param_type" = "flag" ]; then
        echo "FLAGS"
    elif [[ "$model_name" = "resistor" && "$param_description" = "Resistor model default value" ]] \
    ||   [[ "$model_name" = "resistor" && "$param_description" = "Default device length" ]] \
    ||   [[ "$model_name" = "resistor" && "$param_name" = "defw" ]] \
    ||   [[ "$model_name" = "resistor" && "$param_name" = "defl" ]] \
    ||   [[ "$model_name" = "resistor" && "$param_name" = "narrow" ]] \
    ||   [[ "$model_name" = "resistor" && "$param_name" = "short" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_description" = "Capacitor model" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_description" = "Model capacitance" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "defw" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "defl" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "narrow" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "short" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "del" ]] \
    ||   [[ "$model_name" = "inductor" && "$param_description" = "Inductor model" ]] \
    ||   [[ "$model_name" = "ltra" && "$param_name" = "ltra" ]] \
    ||   [[ "$model_name" = "urc" && "$param_name" = "urc" ]] \
    ||   [[ "$model_name" = "transline" && "$param_description" = "Length of line" ]] \
    ||   [[ "$model_name" = "transline" && "$param_name" = "txl" ]] \
    ||   [[ "$model_name" = "transline" && "$param_name" = "pos_node" ]] \
    ||   [[ "$model_name" = "transline" && "$param_name" = "neg_node" ]] \
    ||   grep -iE "R" <<< "$param_flags" >/dev/null
    then
        echo "SUPERFLUOUS"
    elif grep -iE "initial condition" <<< "$param_description" >/dev/null \
    ||   grep -iE "initial voltage" <<< "$param_description" >/dev/null \
    ||   grep -iE "initial current" <<< "$param_description" >/dev/null
    then
        echo "INITIAL_CONDITIONS"
    elif [[ "$model_name" = "ltra" && "$param_name" = "compactrel" ]] \
    ||   [[ "$model_name" = "ltra" && "$param_name" = "compactabs" ]] \
    ||   [[ "$model_name" = "urc" && "$param_name" = "n" ]] \
    ||   [[ "$model_name" = "urc" && "$param_name" = "isperl" ]] \
    ||   [[ "$model_name" = "urc" && "$param_name" = "rsperl" ]] \
    ||   [[ "$model_name" = "urc" && "$param_name" = "compactrel" ]] \
    ||   [[ "$model_name" = "urc" && "$param_name" = "compactabs" ]]
    then
        echo "ADVANCED"
    elif [ "$model_name" = "ltra" ] \
    ||   [ "$model_name" = "tranline" ] \
    ||   [ "$model_name" = "urc" ] \
    ||   [ "$model_name" = "transline" ] \
    ||   [ "$param_name" = "resistance" ] \
    ||   [ "$param_name" = "capacitance" ] \
    ||   [ "$param_name" = "inductance" ] # We make possibly bad exceptions for some instance parameters here.
    then
        echo "PRINCIPAL"
    elif [ "$is_instance_param" = 1 ]; then
        if [[ "$param_name" = "l" || "$param_name" = "w" || "$param_name" = "m" ]]; then
            echo "GEOMETRY"
        else
            # Discard the instance parameters.
            echo "SUPERFLUOUS"
        fi
    elif grep -iE "_max$" <<< "$param_name" >/dev/null; then
        if [ "$is_instance_param" = 0 ]; then
            echo "LIMITING"
        else
            # Discard the instance parameters.
            echo "SUPERFLUOUS"
        fi
    elif [[ "$model_name" = "resistor" && "$param_name" = "rsh" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "di" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "thick" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "cj" ]] \
    ||   [[ "$model_name" = "capacitor" && "$param_name" = "cjsw" ]]
    then
        echo "GEOMETRY"
    elif grep -iE "temperature" <<< "$param_description" >/dev/null \
    ||   grep -iE "temp\. coeff" <<< "$param_description" >/dev/null \
    ||   grep -iE "thermal resistance" <<< "$param_description" >/dev/null
    then
        echo "TEMPERATURE"
    elif grep -iE "noise" <<< "$param_description" >/dev/null; then
        echo "NOISE"
    else
        if [ "$is_instance_param" = 0 ]; then
            echo "DC"
        else
            echo "SUPERFLUOUS"
        fi
    fi
}


run_ngspice()
{
    ngspice -n 2>/dev/null << END
        *
        $1
        .control
        $2
        .endc
END
}


echo_head()
{
    echo "// Generated by the $(basename $0) script."
    echo "// Modify that script instead of this file if you want to make changes."
    echo ""
    echo "#include <sim/ngspice.h>"
    echo ""
}


{
    echo_head

    echo "enum class NGSPICE::MODEL_TYPE"
    echo "{"
    echo "    NONE,"

    echo "$MODELS" | while read -r model_name model_primitive model_level model_version; do
        if [ -n "$model_name" ]; then
            echo "    ${model_name^^},"
        fi
    done

    echo "};"

} > $(dirname "$0")/ngspice_models.h

{
    echo_head

    echo "// We cannot use designated initializers until we upgrade to C++20 (we're C++17 now),"
    echo "// so we do this instead."
    echo "static auto U()    { NGSPICE::PARAM_FLAGS p; p.uninteresting=true; return p; }"
    echo "static auto UR()   { NGSPICE::PARAM_FLAGS p; p.uninteresting=true; p.redundant=true; return p; }"
    echo "static auto P()    { NGSPICE::PARAM_FLAGS p; p.principal=true; return p; }"
    echo "static auto PR()   { NGSPICE::PARAM_FLAGS p; p.principal=true; p.redundant=true; return p; }"
    echo "static auto A()    { NGSPICE::PARAM_FLAGS p; p.ac=true; return p; }"
    echo "static auto AR()   { NGSPICE::PARAM_FLAGS p; p.ac=true; p.redundant=true; return p; }"
    echo "static auto AU()   { NGSPICE::PARAM_FLAGS p; p.ac=true; p.uninteresting=true; return p; }"
    echo "static auto AP()   { NGSPICE::PARAM_FLAGS p; p.ac=true; p.principal=true; return p; }"
    echo "static auto APR()  { NGSPICE::PARAM_FLAGS p; p.ac=true; p.principal=true; p.redundant=true; return p; }"
    echo "static auto AA()   { NGSPICE::PARAM_FLAGS p; p.acOnly=true; return p; }"
    echo "static auto AAU()  { NGSPICE::PARAM_FLAGS p; p.acOnly=true; p.uninteresting=true; return p; }"
    echo "static auto PAA()  { NGSPICE::PARAM_FLAGS p; p.acOnly=true; p.principal=true; return p; }"
    echo "static auto N()    { NGSPICE::PARAM_FLAGS p; p.noise=true; return p; }"
    echo "static auto R()    { NGSPICE::PARAM_FLAGS p; p.redundant=true; return p; }"
    echo "static auto X()    { NGSPICE::PARAM_FLAGS p; p.nonsense=true; return p; }"
    echo "static auto XR()   { NGSPICE::PARAM_FLAGS p; p.nonsense=true; p.redundant=true; return p; }"
    echo "static auto XU()   { NGSPICE::PARAM_FLAGS p; p.nonsense=true; p.uninteresting=true; return p; }"
    echo "static auto Q()    { NGSPICE::PARAM_FLAGS p; p.setQuery=true; return p; }"
    echo "static auto QR()   { NGSPICE::PARAM_FLAGS p; p.setQuery=true; p.redundant=true; return p; }"
    echo "static auto QU()   { NGSPICE::PARAM_FLAGS p; p.setQuery=true; p.uninteresting=true; return p; }"
    echo "static auto Z()    { NGSPICE::PARAM_FLAGS p; p.chkQuery=true; return p; }"
    echo "static auto ZR()   { NGSPICE::PARAM_FLAGS p; p.chkQuery=true; p.redundant=true; return p; }"
    echo "static auto ZU()   { NGSPICE::PARAM_FLAGS p; p.chkQuery=true; p.uninteresting=true; return p; }"
    echo "static auto QO()   { NGSPICE::PARAM_FLAGS p; p.orQuery=true; return p; }"
    echo "static auto QOR()  { NGSPICE::PARAM_FLAGS p; p.orQuery=true; p.redundant=true; return p; }"
    echo ""

    echo "NGSPICE::MODEL_INFO NGSPICE::GetModelInfo( NGSPICE::MODEL_TYPE aType )"
    echo "{"
    echo "    switch( aType )"
    echo "    {"
    echo "    case NGSPICE::MODEL_TYPE::NONE:"
    echo "        return {};"

    echo "$MODELS" | while read -r model_name \
                                   model_type1 \
                                   model_type2 \
                                   model_primitive \
                                   model_pin_count \
                                   model_level \
                                   model_version
    do
        if [ -n "$model_name" ]; then
            # Print model description.
            run_ngspice "" "devhelp -type -flags $model_name" | while read -r name \
                                                                              sep \
                                                                              description
            do
                if [ "$sep" = "-" ]; then
                    echo -n "    case NGSPICE::MODEL_TYPE::${model_name^^}:"
                    echo -n " return { \"$name\","

                    for model_type in "$model_type1" "$model_type2"; do
                        if [ "$model_type" != "-" ]; then
                            echo -n " \"$model_type\","
                        else
                            echo -n " \"\","
                        fi
                    done

                    echo " \"$description\","
                fi
            done

            is_instance_param=0

            # Print model parameter ID, name, direction, type, unit, and description.
            run_ngspice "" "devhelp -type -flags $model_name" | while read -r param_id \
                                                                              param_name \
                                                                              param_dir \
                                                                              param_type \
                                                                              param_flags \
                                                                              param_description
            do
                #&&   [ -n "$param_description" ]
                if [ "$param_id" = "Model" ] && [ "$param_name" = "Parameters" ]; then
                    echo "        // Model parameters"
                    echo "        {"
                elif [ "$param_id" = "Instance" ] && [ "$param_name" = "Parameters" ]; then
                    echo "        },"
                    echo "        // Instance parameters"
                    echo "        {"
                    is_instance_param=1
                elif [ "$param_id" -eq "$param_id" ] 2>/dev/null \
                &&   [ -n "$param_name" ] \
                &&   [ -n "$param_dir" ]
                then
                    echo -n "            { \"${param_name,,}\","
                    echo -n " $param_id,"
                    echo -n " NGSPICE::PARAM_DIR::${param_dir^^},"
                    echo -n " NGSPICE::PARAM_TYPE::${param_type^^},"

                    if [ "$param_flags" != "-" ]; then
                        echo -n " $param_flags(),"
                    else
                        echo -n " {},"
                    fi


                    unit=""

                    # Non-reals are unlikely to have units.
                    if [ "$param_type" = "real" ]; then
                        # Simple finite state machine to read the UNITS string line by line.
                        # Maybe an if-elif-else statement would have been better?

                        # Don't use a pipe here because it creates a subshell, preventing the
                        # changes to the variables from propagating upwards. Bash is cursed.
                        while read -r pattern; do
                            if [ "$unit" = "" ]; then
                                unit="$pattern"
                            elif [ -z "$pattern" ]; then
                                unit=""
                            elif grep -iE "$pattern" <<< "$param_description" >/dev/null; then
                                break
                            fi
                        done <<< "$UNITS"
                    fi

                    if [ "$unit" = "-" ]; then
                        unit=""
                    fi

                    echo -n " \"$unit\","
                    echo -n " NGSPICE::PARAM_CATEGORY::"$(category "$model_name" "$param_name" "$param_flags" "$param_description" "$is_instance_param")","


                    for model_type in "$model_type1" "$model_type2"; do
                        if [ "$model_type" = "-" ]; then
                            echo -n " \"\","
                            continue
                        fi

                        # For a given primitive, Ngspice determines the device model to be used
                        # from two parameters: "level" and "version".
                        params=""

                        if [ "$model_level" != 0 ]; then
                            params="$params level=$model_level"
                        fi

                        if [ "$model_version" != 0 ]; then
                            params="$params version=$model_version"
                        fi

                        netlist=$(cat << END
                            .model $model_type $model_type($params)
                            ${model_primitive}1 $(seq -s ' ' $model_pin_count) $model_type
END
                        )

                        control=$(cat << END
                            op
                            showmod ${model_primitive}1 : $param_name
END
                        )

                        was_model_line=0
                        was_echoed=0
                        
                        # Again don't use a pipe.
                        while read -r name value; do
                            # Ngspice displays only the first 11 characters of the variable name.
                            # We also convert to lowercase because a few parameter names have
                            # uppercase characters in them.

                            lowercase_name=${name,,}
                            lowercase_param_name=${param_name,,}

                            if [ "$was_model_line" = 0 ] && [ "$lowercase_name" = "model" ]; then
                                was_model_line=1
                            elif [ "$was_model_line" = 1 ] \
                                && [ "$lowercase_name" = "${lowercase_param_name:0:11}" ]
                            then
                                if [ "$value" = "<<NAN, error = 7>>" ]; then
                                    value="NaN"
                                elif [ "$value" = "?????????" ]; then
                                    value=""
                                fi

                                was_echoed=1
                                echo -n " \"$value\","
                            fi
                        done < <(run_ngspice "$netlist" "$control")

                        #if [ "$was_echoed" = 0 ]; then
                            #echo ""
                            #echo "Error! Default value not found."
                            #exit 1
                        #fi
                    done

                    echo " \"$param_description\" },"
                fi
            done


            echo "        } };"
        fi
    done

    echo "    }"
    echo ""

    echo "    wxFAIL;"
    echo "    return {};"
    echo "}"
} > $(dirname "$0")/new_ngspice_models.cpp
