if [ $# -ne 1 ]; then
    echo "Usage: ./run.sh [thread number]" >&2; exit 1
else
    regexp_integer='^[0-9]+$'
    if ! [[ $1 =~ $regexp_integer ]] ; then
        echo "Error: [thread number] must be an integer" >&2; exit 1
    elif [ $1 -eq 0 ]; then
        echo "Error: thread number cannot be zero" >&2; exit 1
    else
    OMP_NUM_THREADS=$1 ./hello
    fi
fi
