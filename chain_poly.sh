#!/bin/bash


Valve()
{
    out=$(mktemp)
    cat "${PipeFile}" "${1}" |head -n -1  |${PROG} - >${out}
    rm "${PipeFile}"
    PipeFile=${out}
    if tail -1 "${1}" |grep -e '^STOP$' >/dev/null 2>&1; then
        cat "${out}"
        rm "${PipeFile}"
    else
        next=$(cat ${1} |tail -1 |sed 's/FILE //')
        Valve "${next}"
    fi
}


if [[ ! "$#" -eq 2 ]]; then
    echo Zła liczba parametrów
    exit 1
fi

if [[ -x "${1}" ]]; then
    PROG=$(realpath "${1}")
else
    echo Podany program nie jest plikiem wykonywalnym
    exit 1
fi

if [[ -d "${2}" ]]; then
    cd "${2}"
else
    echo Podany katalog nie jest katalogiem
    exit 1
fi

PipeFile=$(mktemp)
MyTemp=$(mktemp)
tail +2 "$(grep -le '^START$' *)" >${MyTemp}
Valve "${MyTemp}"
rm "${MyTemp}"
