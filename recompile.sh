make clean

rm -rf SS_1/ss.out 
rm -rf SS_2/ss.out 
rm -rf SS_3/ss.out 

make 

cp ss.out SS_1/ss.out
cp ss.out SS_2/ss.out
cp ss.out SS_3/ss.out
