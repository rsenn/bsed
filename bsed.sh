COUNTER=${TOOLS:-/opt/exp}/bin/expcounter
if [ -x $COUNTER ]
then
    $COUNTER -p bsed -n bsed
fi
exec ${TOOLS:-/opt/exp}/lib/bsed/bsed ${1+"$@"}
