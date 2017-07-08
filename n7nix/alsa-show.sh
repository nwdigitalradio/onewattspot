#!/bin/bash

CARD="udrc"
#CARD="Generic"

# amixer -c $CARD get 'ADC Level'
#amixer -c $CARD get 'LO Driver Gain'
CONTROL_LIST="'ADC Level''LO Drive Gain' 'PCM'"

control="PCM"
PCM_STR="$(amixer -c $CARD get "$control" | grep -i "Simple mixer control")"
#echo "PCM: $PCM_STR"
PCM_VAL=$(amixer -c $CARD get "$control" | grep -i -m 1 "db")
PCM_VAL_L=${PCM_VAL##* }
#echo "PCM_VAL: Left $PCM_VAL"
PCM_VAL=$(amixer -c $CARD get "$control" | grep -i -m 2 "db" | tail -n 1 | cut -d ' ' -f5-)
PCM_VAL_R=${PCM_VAL##* }
#echo "PCM_VAL: RIght $PCM_VAL"
printf "%s\t        L:%s, R:%s\n" $control $PCM_VAL_L $PCM_VAL_R

control="ADC Level"
PCM_STR="$(amixer -c $CARD sget "$control" | grep -i "Simple mixer control")"
#echo "$control: $PCM_STR"
PCM_VAL=$(amixer -c $CARD get "$control" | grep -i -m 1 "db")
PCM_VAL_L=${PCM_VAL##* }
#echo "PCM_VAL: Left $PCM_VAL"
PCM_VAL=$(amixer -c $CARD get "$control" | grep -i -m 2 "db" | tail -n 1 | cut -d ' ' -f5-)
PCM_VAL_R=${PCM_VAL##* }
#echo "PCM_VAL: RIght $PCM_VAL"
#echo "$control   L:$PCM_VAL_L, R:$PCM_VAL_R"
printf "%s\tL:%s, R:%s\n" "$control" $PCM_VAL_L $PCM_VAL_R

control="LO Driver Gain"
PCM_STR="$(amixer -c $CARD get "$control" | grep -i "Simple mixer control")"
#echo "PCM: $PCM_STR"
PCM_VAL=$(amixer -c $CARD get "$control" | grep -i -m 1 "db")
PCM_VAL_L=${PCM_VAL##* }
#echo "PCM_VAL: Left $PCM_VAL"
PCM_VAL=$(amixer -c $CARD get "$control" | grep -i -m 2 "db" | tail -n 1 | cut -d ' ' -f5-)
PCM_VAL_R=${PCM_VAL##* }
#echo "PCM_VAL: RIght $PCM_VAL"
#echo "$control   L:$PCM_VAL_L, R:$PCM_VAL_R"
printf "%s  L:%s, R:%s\n" "$control" $PCM_VAL_L $PCM_VAL_R


