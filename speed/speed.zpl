
# ---------------------------------------------------------------------------
# sets
# ---------------------------------------------------------------------------

set S := { read "sites.dat" as "<1s>" comment "#" };
set I := { read "installations.dat" as "<1s>" comment "#" };
set M := { read "mobiles.dat" as "<1s>" comment "#" };
set Services := { read "services.dat" as "<1s>" comment "#" };
set IM := I * M;

# ---------------------------------------------------------------------------
# PARAMETERS
# ---------------------------------------------------------------------------

set Names                := { "SCALING_FACTOR",
                              "MIN_PERMISSIBLE_ATTENUATION", 
                              "NOISE_RISE", 
                              "CUTOFF_CIR_PL_UL", 
                              "CUTOFF_CIR_PL_DL" };
param parameter[Names]   := read "parameter.dat" as "<1s> 2n" comment "#";

param min_inst    [S]    := read "sites.dat" as "<1s> 2n" comment "#";
param max_inst    [S]    := read "sites.dat" as "<1s> 3n" comment "#";
param cost_s_A    [S]    := read "sites.dat" as "<1s> 4n" comment "#";
param force_open  [S]    := read "sites.dat" as "<1s> 5n" comment "#";

param path_loss   [IM]   := read "attenuation.dat" as "<1s,2s> 3n" 
                     comment "#" default 0.0;

param pmin_pilot_A  [I]  := read "installations.dat" as "<1s>  4n" comment "#";
param pmax_pilot_A  [I]  := read "installations.dat" as "<1s>  5n" comment "#";
param pmax_dl_A     [I]  := read "installations.dat" as "<1s>  8n" comment "#";
param at_site       [I]  := read "installations.dat" as "<1s>  2s" comment "#";
param noise_i_A     [I]  := read "installations.dat" as "<1s>  3n" comment "#";
param cchf_i        [I]  := read "installations.dat" as "<1s>  9n" comment "#";
param min_sust_ul_i [I]  := read "installations.dat" as "<1s> 10n" comment "#";
param max_load_dl   [I]  := read "installations.dat" as "<1s> 11n" comment "#";
param force_open_i  [I]  := read "installations.dat" as "<1s> 12n" comment "#";
param loss_I_ul     [I]  := read "installations.dat" as "<1s>  6n" comment "#";
param loss_I_dl     [I]  := read "installations.dat" as "<1s>  7n" comment "#";
param cost_z_A      [I]  := read "installations.dat" as "<1s> 13n" comment "#";
param flavour       [I]  := read "installations.dat" as "<1s> 14s" comment "#";

param snap           [M] := read "mobiles.dat" as "<1s>   2n" comment "#";
param service        [M] := read "mobiles.dat" as "<1s>  16s" comment "#";
param noise_m_A      [M] := read "mobiles.dat" as "<1s>   3n" comment "#";
param pmin_ul_A      [M] := read "mobiles.dat" as "<1s>   4n" comment "#";
param pmax_ul_A      [M] := read "mobiles.dat" as "<1s>   5n" comment "#";
param activity_ul    [M] := read "mobiles.dat" as "<1s>   6n" comment "#";
param activity_dl    [M] := read "mobiles.dat" as "<1s>   7n" comment "#";
param min_rscp_pilot_A[M]:= read "mobiles.dat" as "<1s>   8n" comment "#";
param pilot_min_ecio [M] := read "mobiles.dat" as "<1s>   9n" comment "#";
param cir_ul         [M] := read "mobiles.dat" as "<1s>  10n" comment "#";
param cir_dl         [M] := read "mobiles.dat" as "<1s>  11n" comment "#";
param loss_M_ul      [M] := read "mobiles.dat" as "<1s>  12n" comment "#";
param loss_M_dl      [M] := read "mobiles.dat" as "<1s>  13n" comment "#";
param orthogo        [M] := read "mobiles.dat" as "<1s>  17n" comment "#";

param pmax_link_dl_A [I*Services] := read "power_limits_dl.dat" as "<1s,2s> 3n" 
                   comment "#" default 100.0;

#-- begin scaling --

param factor := parameter["SCALING_FACTOR"];

param cost_s         [<s> in S]            := factor * cost_s_A [s];
param cost_z         [<i> in I]            := factor * cost_z_A [i];

param pmin_pilot     [<i> in I]            := factor * pmin_pilot_A [i];
param pmax_pilot     [<i> in I]            := factor * pmax_pilot_A [i];
param pmax_dl        [<i> in I]            := factor * pmax_dl_A [i];
param noise_i        [<i> in I]            := factor * noise_i_A [i];

param noise_m        [<m> in M]            := factor * noise_m_A [m];
param pmin_ul        [<m> in M]            := factor * pmin_ul_A [m];
param pmax_ul        [<m> in M]            := factor * pmax_ul_A [m];
param min_rscp_pilot [<m> in M]            := factor * min_rscp_pilot_A [m];

param pmax_link_dl   [<i,s> in I*Services] := factor * 
      (if pmax_dl_A[i] - cchf_i[i] * pmin_pilot_A[i] 
          < pmax_link_dl_A [i,s] 
       then
          pmax_dl_A[i] - cchf_i[i] * pmin_pilot_A[i]
       else
          pmax_link_dl_A [i,s]
       end);

#-- end scaling --

#-- to do
# param C1[I]        := read "rating.dat" as "<1s> 4n" comment "#";
# param C2[I]        := read "rating.dat" as "<1s> 6n" comment "#";
# param C3[I]        := read "rating.dat" as "<1s> 7n" comment "#";

#-- to do
# param C1_max       := max <i> in I : C1[i];
# param C2_max       := max <i> in I : C2[i];
# param C3_max       := max <i> in I : C3[i];


param atten_dl  [<i,m> in IM] := path_loss[i,m] / (loss_I_dl[i] * loss_M_dl[m]);
param atten_ul  [<i,m> in IM] := path_loss[i,m] / (loss_M_ul[m] * loss_I_ul[i]);

param max_atten_dl [<m> in M] := max <i> in I : atten_dl[i,m];
param max_atten_ul [<m> in M] := max <i> in I : atten_ul[i,m];

param min_permissible_attenuation := parameter["MIN_PERMISSIBLE_ATTENUATION"];  

# distinct set for mobiles active in uplink and downlink
set M_ul    := { <m> in M | activity_ul[m] > 0 };
set M_dl    := { <m> in M | activity_dl[m] > 0 };
set IM_dl   := I * M_dl;
set IM_ul   := I * M_ul;

set D   := { 0 to max <m> in M : snap[m] };
set ID  := I * D;


param invortho    [<m> in M_dl]   := 1.0 - orthogo[m];

param pi_scale [<i,d> in ID]  
   := max <m> in M_ul with snap[m] == d : atten_ul[i,m] * activity_ul[m];

# end-to-end attenuation

# preparation for tighter definition of serviceable
param coverable [<m> in M] 
      := if     (activity_ul[m] + activity_dl[m] > 0)
            and max_atten_ul[m] >= min_permissible_attenuation
            and max_atten_dl[m] >= min_permissible_attenuation
      then 1 else 0 end;


# 0. attenuation value > 0
# 1. max received pilot has to be greater than cir * noise
# 2. max received pilot has to be greater than RSCP
# 3. only mobiles surviving at maximum noise rise in uplink are served
param noise_rise := parameter["NOISE_RISE"];  
param serviceable [<i,m> in IM]
   := if      coverable [m] == 1
          and atten_dl[i,m] * pmax_pilot[i] 
              >= pilot_min_ecio[m] * noise_rise * noise_m[m] 
          and atten_dl[i,m] * pmax_pilot[i] >= min_rscp_pilot[m]
          and (   activity_dl[m] <= 0
           or atten_dl[i,m] * pmax_link_dl[i,service[m]] 
              >= cir_dl[m] * noise_rise * noise_m[m])
          and (   activity_ul[m] <= 0
           or atten_ul[i,m] * pmax_ul[m] 
              >= cir_ul[m] * min_sust_ul_i[i] * noise_rise * noise_i[i])
      then 1 else 0 end;

param max_pmax_pilot := max <i> in I : pmax_pilot[i];

param theta_ul[<i,d> in ID] := 
   sum <m> in M_ul with snap[m] == d : 
      atten_ul[i,m] * activity_ul[m] * pmax_ul[m];

param theta_dl[<m> in M] := sum <i> in I : atten_dl[i,m] * pmax_dl[i];

param pr_scale[<m> in M] := max <i> in I : atten_dl[i,m];


# ---------------------------------------------------------------------------
# VARIABLES
# ---------------------------------------------------------------------------

var s [<s> in S]        integer    
       >= force_open[s] <=1    priority 100  startval 1.0;
var x [<i,m> in IM]     integer
       <=  serviceable [i,m]   priority  10;
var z [I]               binary    priority  80  startval 1.0;

var pu [<m> in M_ul]    real    # power uplink
       >= pmin_ul[m] * (max <i> in I : serviceable[i,m])
       <= pmax_ul[m];
var pd [<i,m> in IM_dl] real    # power downlink per link 
       <= pmax_link_dl[i,service[m]];   
var pp [<i> in I]       real    # pilot power 
       <= pmax_pilot[i];        # ATTENTION pp has to come after x in this file 
                                # becasue of the order of the zimpl .tbl file!!

var cpp                 real    # common pilot power 
       <= max_pmax_pilot;
var pt [<i,d> in ID]    real    # power total downlink
       <= pmax_dl[i];
var pi [<i,d> in ID]    real;   # total interference power uplink
var pr [<m> in M]       real;   # total power received at m

var aux                 binary;

# ---------------------------------------------------------------------------
# OBJECTIVE
# ---------------------------------------------------------------------------

minimize cost: 
    sum <k>   in S     :  cost_s[k] * s[k] 
 #-- to do
  + sum <i>   in I     :  cost_z[i] * z[i] #(2 * C1[i] + C2[i] + C3[i])/4 * z[i]
  + sum <m>   in M_ul  :  10 * pu[m]
  + sum <i,m> in IM_dl :  20 * pd[i,m]
  + sum <i>   in I     :  5 * pp[i]
  + sum <i,d> in ID    :  10 * pt[i,d]
 ;

# ---------------------------------------------------------------------------
# SITE SELECTION
# ---------------------------------------------------------------------------

# enforce use of mandatory installations if site is used

subto c0: forall <i> in I do z[i] - force_open_i[i] * s[at_site[i]] >= 0;

# Only active sites can have installations

subto c1: forall <i> in I do z[i] - s[at_site[i]] <= 0;

# There may be sites which have a minimum number of cells

subto c2a: forall <k> in S do 
   sum <i> in I with at_site[i] == k : z[i] >= min_inst[k];

# The number of installations per site is limited

subto c2b: forall <k> in S do 
   sum <i> in I with at_site[i] == k : z[i] <= max_inst[k];

# Only active installations may serve mobiles

subto c3: forall <i,m> in IM with serviceable[i,m] == 1 do x[i,m] - z[i] <= 0;

# Mobiles have to be served

subto c4: forall <m> in M do 
   sum <i> in I with serviceable[i,m] == 1 : x[i,m] 
   == 
   max <i> in I : serviceable[i,m]
   ;

# ---------------------------------------------------------------------------
# UPLINK
# ---------------------------------------------------------------------------

# Received power (interference) at base station

subto c5: forall <i,d> in ID do
   sum <m> in M_ul with snap[m] == d : 
      atten_ul[i,m] * activity_ul[m] * pu[m] - pi_scale[i,d] * pi[i,d] == 0
   , scale
   ;

# CIR target

subto c6: 
   forall <i,m> in IM_ul with serviceable[i,m] == 1 do
      - (atten_ul[i,m] / cir_ul[m]) * pu[m] 
# --- phi ---
      + pi_scale[i,snap[m]] * pi[i,snap[m]] 
      - atten_ul[i,m] * activity_ul[m] * pu[m]
      + noise_i[i] * x[i,m]
# --- theta ---
      + theta_ul[i,snap[m]] * x[i,m]
   <= 
      theta_ul[i,snap[m]]
   , scale
   ;

# ---------------------------------------------------------------------------
# DOWNLINK
# ---------------------------------------------------------------------------

# Minimum and maximum downlink power must be used if mobile is served

# However, no minimum available

# subto c7a: forall <i,m> in IM_dl with serviceable[i,m] == 1 do
#    pmin_link[i] * x[i,m] - pd[i,m] <= 0
#   , separate
#    ;

subto c7b: forall <i,m> in IM_dl with serviceable[i,m] == 1 do 
    pmax_link_dl[i,service[m]] * x[i,m] - pd[i,m] >= 0
#   , separate
    ;

# Minimum and maximum pilot power must be used if installation is used

subto c8a: forall <i> in I do pmin_pilot[i] * z[i] <= pp[i];
subto c8b: forall <i> in I do pmax_pilot[i] * z[i] >= pp[i];
subto c8c: forall <i> in I do pp[i] <= cpp;
subto c8d: forall <i> in I with flavour[i]=="macro" 
               do pp[i] >= cpp - max_pmax_pilot * (1 - z[i]);

# Limit the maximum power per installation

subto c9a: forall <i,d> in ID do 
   pt[i,d] == (1.0 + cchf_i[i]) * pp[i] 
   + sum <m> in M_dl with snap[m] == d and serviceable[i,m] == 1 : 
       activity_dl[m] * pd[i,m]
#   , separate
    ;

subto c9b: forall <i,d> in ID do 
   pt[i,d] <= max_load_dl[i] * pmax_dl[i] * z[i];

# compute total received power at mobile m

subto c10: forall <m> in M do 
   pr_scale[m] * pr[m] == sum <i> in I : atten_dl[i,m] * pt[i,snap[m]] 
   , scale 
   ;

# CIR downlink
# atten/cir * pwr -  phi' - theta * x >= noise - theta
# note: cir * noise is also part of theta and cancelled

subto c11: 
   forall <i,m> in IM_dl with serviceable[i,m] == 1 do
      - (atten_dl[i,m] / cir_dl[m]) * pd[i,m]
# --- (phi') 
      + invortho[m] * atten_dl[i,m] * pt[i,snap[m]] 
      - invortho[m] * atten_dl[i,m] * activity_dl[m] * pd[i,m]
      + pr_scale[m] * pr[m] - atten_dl[i,m] * pt[i,snap[m]] 
# --- (theta x_mi)
      + invortho[m] * atten_dl[i,m] * pmax_dl[i] * x[i,m]
      + theta_dl[m] * x[i,m]
      + noise_m[m] * x[i,m]
   <= 
# --- (theta - noise)
      + invortho[m] * atten_dl[i,m] * pmax_dl[i]
      + theta_dl[m]
   , scale
#  , separate
   ;
   
# ---------------------------------------------------------------------------
# DOWNLINK PILOT
# ---------------------------------------------------------------------------

# Ec/Io Pilot

subto c12: 
   forall <i,m> in IM with serviceable[i,m] == 1 do
      - (atten_dl[i,m] / pilot_min_ecio[m]) * pp[i]
# --- (phi') 
      + pr_scale[m] * pr[m]
      - atten_dl[i,m] * pp[i]
# --- (theta x_mi)
      + theta_dl[m] * x[i,m]
      + noise_m[m] * x[i,m]
   <= 
# --- (theta - noise)
      + theta_dl[m]
   , scale
#  , separate
   ;

# Minimum Pilot RSCP

subto c13: forall <i,m> in IM 
       with 
        serviceable[i,m] == 1 
       and 
        # only write inequality if it can be violated
        atten_dl[i,m] * pmin_pilot[i] < min_rscp_pilot[m]
    do
   atten_dl[i,m] * pp[i] >= min_rscp_pilot[m] * x[i,m]
   , scale
#  , separate
   ;

# ---------------------------------------------------------------------------
# CUTS
# ---------------------------------------------------------------------------

# MIR-cut based on uplink CIR target inequality

subto m1: forall <i,m> in IM_ul with serviceable[i,m] == 1 do
  + 1 * x[i,m]
  - atten_ul[i,m] * (1 / cir_ul[m] + activity_ul[m]) / noise_i[i]
  * pu[m] 
  <= 0
  , scale
# , separate 
  ;

# MIR-cut based on downlink CIR target inequality

subto m2: forall <i,m> in IM_dl with serviceable[i,m] == 1 do
  + 1 * x[i,m]
  - atten_dl[i,m] * (1 / cir_dl[m] + invortho[m] * activity_dl[m]) 
  / noise_m[m] * pd[i,m] 
  <= 0
  , scale
#   ,separate   
  ;

# ---------------------------------------------------------------------------
# HEURISTIC CONSTRAINTS
# ---------------------------------------------------------------------------

# Best server

subto bs1: 
   forall <i,m> in IM with serviceable[i,m] == 1 do 
      forall <j> in I with j != i
     and serviceable[j,m] == 1
         and atten_dl[j,m] > 2 * atten_dl[i,m]
         and atten_ul[j,m] > 2 * atten_ul[i,m]
         do
            z[j] + x[i,m] <= 1
#  , separate
   ;

subto count:
   sum <m> in M : (max <i> in I : serviceable[i,m]) * aux
   <= sum <m> in M : 1;
