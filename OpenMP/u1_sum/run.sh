if [ $# -ne 2 ]; then
    echo "Usage: ./run.sh [thread number] [N]" >&2; exit 1
else
    regexp_integer='^[0-9]+$'
    # Check thread number
    if ! [[ $1 =~ $regexp_integer ]] ; then
        echo "Error: [thread number] must be an integer" >&2; exit 1
    elif [ $1 -eq 0 ]; then
        echo "Error: thread number cannot be zero" >&2; exit 1
    fi
    #Check sum threshold
    if ! [[ $2 =~ $regexp_integer ]] ; then
        echo "Error: sum threshold must be a non-negative integer" >&2; exit 1
    else
        OMP_NUM_THREADS=$1 ./sum $2
    fi
fi
