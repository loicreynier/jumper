# Common to bash and zsh
export __JUMPER_FOLDERS=~/.jfolders
export __JUMPER_FILES=~/.jfiles
export __JUMPER_MAX_RESULTS=150

[[ -f ${__JUMPER_FOLDERS} ]] || touch "${__JUMPER_FOLDERS}"
[[ -f ${__JUMPER_FILES} ]] || touch "${__JUMPER_FILES}"

# Jump to most frecent folder
z() {
    args="${@// /\ }"
	new_path=$(jumper -f "${__JUMPER_FOLDERS}" -n 1 "${args}")
	if [[ -z $new_path ]]; then
		echo 'No match found.'
	else
		cd "${new_path}"
	fi
}

# Edit the most frecent file
zf() {
    args="${@// /\ }"
    file=$(jumper -f "${__JUMPER_FILES}" -n 1 "${args}")
	if [[ -z $file ]]; then
		echo 'No match found.'
    else
		eval "${EDITOR} '${file}'"
	fi
}

# Fuzzy-find directories
__jumper_fdir() {
	__JUMPER="jumper -c -f ${__JUMPER_FOLDERS} -n ${__JUMPER_MAX_RESULTS}"
	fzf --height=70% --layout=reverse \
        --keep-right \
		--ansi --disabled --query '' \
		--bind "start:reload:${__JUMPER} {q}" \
		--bind "change:reload:sleep 0.05; ${__JUMPER} {q} || true"
}

# Fuzzy-find files
__jumper_ffile() {
	__JUMPER="jumper -c -f ${__JUMPER_FILES} -n ${__JUMPER_MAX_RESULTS}"
	fzf --height=70% --layout=reverse \
        --keep-right \
		--ansi --disabled --query '' \
		--bind "start:reload:${__JUMPER} {q}" \
		--bind "change:reload:sleep 0.05; ${__JUMPER} {q} || true"
}


# Add folder to database
__jumper_update_db() {
    jumper -f "${__JUMPER_FOLDERS}" -a "$PWD"
}

# Clean simlinks
__jumper_delete_duplicate_symlinks(){
    tempfile="${__JUMPER_FOLDERS}_temp"
    cp "${__JUMPER_FOLDERS}" "${tempfile}"
    while IFS= read -r line ; do
        entry="${line%%|*}"
        real=$(realpath $entry)
        if [[ $entry != $real ]]; then
            sed -i -e "\:^${real}|:d" "${tempfile}"
        fi
    done < "${__JUMPER_FOLDERS}"
    mv "${tempfile}" "${__JUMPER_FOLDERS}"
}

__jumper_replace_symlinks(){
    tempfile="${__JUMPER_FOLDERS}_temp"
    cp "${__JUMPER_FOLDERS}" "${tempfile}"
    while IFS= read -r line ; do
        entry="${line%%|*}"
        real=$(realpath $entry)
        if [[ $entry != $real ]]; then
            sed -i -e "s:^${real}/:${entry}/:g" "${tempfile}"
        fi
    done < "${__JUMPER_FOLDERS}"
    mv "${tempfile}" "${__JUMPER_FOLDERS}"
}

__jumper_clean_symlinks(){
    __jumper_delete_duplicate_symlinks
    __jumper_replace_symlinks
}

# Remove entries for which the folder does not exist anymore
__jumper_clean_folders_db() {
    tempfile="${__JUMPER_FOLDERS}_temp"
    [[ -f ${tempfile} ]] && rm ${tempfile}
    while IFS= read -r line ; do
        entry="${line%%|*}"
        if [[ -d $entry ]]; then
            echo "$line" >> ${tempfile}
        else
            echo "Removing $entry from database."
        fi
    done < "${__JUMPER_FOLDERS}"
    mv "${tempfile}" "${__JUMPER_FOLDERS}"
}

# Remove entries for which the file does not exist anymore
__jumper_clean_files_db() {
    tempfile="${__JUMPER_FILES}_temp"
    [[ -f ${tempfile} ]] && rm ${tempfile}
    while IFS= read -r line ; do
        entry="${line%%|*}"
        if [[ -f $entry ]]; then
            echo "$line" >> ${tempfile}
        else
            echo "Removing $entry from database."
        fi
    done < "${__JUMPER_FILES}"
    mv "${tempfile}" "${__JUMPER_FILES}"
}

# For Bash
run-fz() {
    selected=$(__jumper_fdir)
    pre="${READLINE_LINE:0:$READLINE_POINT}"
    if [[ -z $pre ]] && [[ ! -z ${selected} ]]; then
        cd "$selected"
    else
        READLINE_LINE="${pre}$selected${READLINE_LINE:$READLINE_POINT}"
        READLINE_POINT=$(( READLINE_POINT + ${#selected} ))
    fi
}
run-fz-file() {
    selected=$(__jumper_ffile)
    pre="${READLINE_LINE:0:$READLINE_POINT}"
    READLINE_LINE="${pre}$selected${READLINE_LINE:$READLINE_POINT}"
    READLINE_POINT=$(( READLINE_POINT + ${#selected} ))
}

# Bindings
bind -x '"\C-y": run-fz'
# In some terminals, C-u is bind to kill
# and can not be remapped.
stty kill undef
bind -x '"\C-u": run-fz-file'

# Update db
PROMPT_COMMAND="__jumper_update_db;$PROMPT_COMMAND"
