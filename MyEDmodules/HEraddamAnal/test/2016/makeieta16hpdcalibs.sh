#!/bin/bash
# Run in the _HE16d3, _HE-16d3 directories
#
for day in 113 171 187 199 220 241 253 287 302
do
grep -h ",1,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day >hpd23calib4ietaGE21_day$day.txt
grep -h ",4,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",5,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",8,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",9,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",12," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",13," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",16," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",17," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",20," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",21," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",24," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",25," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",28," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",29," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",32," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",33," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",36," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",37," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",40," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",41," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",44," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",45," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
#
# filename marked ".txt.bad" for HE -16.
#
grep -h ",48," fcamps2016_11*     | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
#
grep -h ",49," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",52," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",53," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",56," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",57," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",60," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",61," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",64," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",65," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",68," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",69," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt
grep -h ",72," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd23calib4ietaGE21_day$day.txt

grep -h ",2,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >hpd14calib4ietaGE21_day$day.txt
grep -h ",3,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",6,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",7,"  fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",10," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",11," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",14," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",15," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",18," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",19," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",22," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",23," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",26," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",27," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",30," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",31," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",34," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",35," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",38," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",39," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",42," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",43," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",46," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",47," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",50," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",51," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",54," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",55," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",58," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",59," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",62," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",63," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",66," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",67," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",70," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
grep -h ",71," fcamps2016_11*.txt | awk '{if ($2 == dayofyear) print}' dayofyear=$day  >>hpd14calib4ietaGE21_day$day.txt
done
