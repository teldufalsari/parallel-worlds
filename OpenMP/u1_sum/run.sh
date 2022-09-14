if [ $# -ne 2 ]; then
    echo "Usage: ./run.sh [thread number] [N]" >&2; exit 1
else
    regexp_integer='^[0-9]+$'
    # Check number of threads
    if ! [[ $1 =~ $regexp_integer ]] ; then
        echo "Error: [thread number] must be an integer" >&2; exit 1
    elif [ $1 -eq 0 ]; then
        echo "Error: thread number cannot be zero" >&2; exit 1
    #Check sum limit
    elif ! [[ $2 =~ $regexp_integer ]] ; then
        echo "Error: sum threshold must be a non-negative integer" >&2; exit 1
    else
        echo "OMP_NUM_THREADS=$1"
        OMP_NUM_THREADS=$1 ./sum $2
    fi
fi
