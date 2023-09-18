
set -xe

OUT=data.csv

rm $OUT
chunks=(0 1 2 4 8 16 32 64 128 256, 512)
for c in "${chunks[@]}"
do
    for i in {1..12..1}
    do
    OMP_NUM_THREADS=$i ./pardith ./images/mandrill.pgm $c >> $OUT
    done
done
