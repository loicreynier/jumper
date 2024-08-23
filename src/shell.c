#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char bash_variables[] =
    "[[ -n $__JUMPER_FLAGS ]] || __JUMPER_FLAGS='-cHo -n 500'\n"
    "[[ -n $__JUMPER_FZF_OPTS ]] || __JUMPER_FZF_OPTS='--height=70%% "
    "--layout=reverse --keep-right --preview-window=hidden --ansi'\n"
    "if [[ -z $__JUMPER_FZF_FILES_PREVIEW ]]; then\n"
    "    if [[ -n $(which bat) ]]; then\n"
    "        __JUMPER_FZF_FILES_PREVIEW='bat --color=always'\n"
    "    else\n"
    "        __JUMPER_FZF_FILES_PREVIEW='cat'\n"
    "    fi\n"
    "fi\n"
    "[[ -n $__JUMPER_FZF_FOLDERS_PREVIEW ]] || "
    "__JUMPER_FZF_FOLDERS_PREVIEW='ls -1UpC --color=always'\n"
    "[[ -n $__JUMPER_TOGGLE_PREVIEW ]] || __JUMPER_TOGGLE_PREVIEW='ctrl-p'\n";

static const char bash_functions[] =
    "z() {\n"
    "    args=\"${@// /\\ }\"\n"
    "	new_path=$(jumper find --type=directories -n 1 \"${args}\")\n"
    "	if [[ -z $new_path ]]; then\n"
    "		echo 'No match found.'\n"
    "	else\n"
    "		cd \"${new_path}\"\n"
    "	fi\n"
    "}\n"
    "zf() {\n"
    "    args=\"${@// /\\ }\"\n"
    "    file=$(jumper find --type=files -n 1 \"${args}\")\n"
    "	if [[ -z $file ]]; then\n"
    "		echo 'No match found.'\n"
    "    else\n"
    "		eval \"${EDITOR} '${file}'\"\n"
    "	fi\n"
    "}\n"
    "__jumper_fdir() {\n"
    "	__JUMPER=\"jumper find --type=directories ${__JUMPER_FLAGS}\"\n"
    "	fzf ${__JUMPER_FZF_OPTS} --disabled --query \"$1\" \\\n"
    "        --preview \"eval x={}; ${__JUMPER_FZF_FOLDERS_PREVIEW} \\$x\" \\\n"
    "        --bind \"${__JUMPER_TOGGLE_PREVIEW}:toggle-preview\" \\\n"
    "		--bind \"start:reload:${__JUMPER} {q}\" \\\n"
    "		--bind \"change:reload:sleep 0.05; ${__JUMPER} {q} || true\"\n"
    "}\n"
    "__jumper_ffile() {\n"
    "	__JUMPER=\"jumper find --type=files ${__JUMPER_FLAGS}\"\n"
    "	fzf ${__JUMPER_FZF_OPTS} --disabled --query \"$1\" \\\n"
    "        --preview \"eval x={}; ${__JUMPER_FZF_FILES_PREVIEW} \\$x\" \\\n"
    "        --bind \"${__JUMPER_TOGGLE_PREVIEW}:toggle-preview\" \\\n"
    "		--bind \"start:reload:${__JUMPER} {q}\" \\\n"
    "		--bind \"change:reload:sleep 0.05; ${__JUMPER} {q} || true\"\n"
    "}\n"
    "zi() {\n"
    "	new_path=$(__jumper_fdir)\n"
    "	if [[ -n $new_path ]]; then\n"
    "		cd \"${new_path/#\\~/$HOME}\"\n"
    "	fi\n"
    "}\n"
    "zfi() {\n"
    "    file=$(__jumper_ffile)\n"
    "	if [[ -n $file ]]; then\n"
    "		$EDITOR \"${file/#\\~/$HOME}\"\n"
    "	fi\n"
    "}\n"
    "__jumper_delete_duplicate_symlinks(){\n"
    "    tempfile=\"${__JUMPER_FOLDERS}_temp\"\n"
    "    cp \"${__JUMPER_FOLDERS}\" \"${tempfile}\"\n"
    "    while IFS= read -r line ; do\n"
    "        entry=\"${line%%%%|*}\"\n"
    "        real=$(realpath $entry)\n"
    "        if [[ $entry != $real ]]; then\n"
    "            sed -i -e \"\\:^${real}|:d\" \"${tempfile}\"\n"
    "        fi\n"
    "    done < \"${__JUMPER_FOLDERS}\"\n"
    "    mv \"${tempfile}\" \"${__JUMPER_FOLDERS}\"\n"
    "}\n"
    "__jumper_replace_symlinks(){\n"
    "    tempfile=\"${__JUMPER_FOLDERS}_temp\"\n"
    "    cp \"${__JUMPER_FOLDERS}\" \"${tempfile}\"\n"
    "    while IFS= read -r line ; do\n"
    "        entry=\"${line%%%%|*}\"\n"
    "        real=$(realpath $entry)\n"
    "        if [[ $entry != $real ]]; then\n"
    "            sed -i -e \"s:^${real}/:${entry}/:g\" \"${tempfile}\"\n"
    "        fi\n"
    "    done < \"${__JUMPER_FOLDERS}\"\n"
    "    mv \"${tempfile}\" \"${__JUMPER_FOLDERS}\"\n"
    "}\n"
    "__jumper_clean_symlinks(){\n"
    "    __jumper_delete_duplicate_symlinks\n"
    "    __jumper_replace_symlinks\n"
    "}\n"
    "_jumper_clean() {\n"
    "    (jumper clean --type=files > /dev/null 2>&1 && jumper clean "
    "--type=directories > /dev/null 2>&1 &)\n"
    "}\n"
    "__jumper_update_db() {\n"
    "    if [[ ! -z $__jumper_current_folder ]]; then\n"
    "        if [[ $__jumper_current_folder != $PWD ]]; then\n"
    "            jumper update --type=directories -w 1.0 \"$PWD\"\n"
    "        else\n"
    "            jumper update --type=directories -w 0.3 \"$PWD\"\n"
    "        fi\n"
    "    fi\n"
    "    __jumper_current_folder=$PWD\n"
    "    if [[ -n $__JUMPER_CLEAN_FREQ ]] && [[ $(( RANDOM %% "
    "__JUMPER_CLEAN_FREQ )) == 0 ]]; then\n"
    "        _jumper_clean\n"
    "    fi\n"
    "}\n"
    "jumper-find-dir() {\n"
    "    selected=$(__jumper_fdir)\n"
    "    pre=\"${READLINE_LINE:0:$READLINE_POINT}\"\n"
    "    READLINE_LINE=\"${pre}$selected${READLINE_LINE:$READLINE_POINT}\"\n"
    "    READLINE_POINT=$(( READLINE_POINT + ${#selected} ))\n"
    "}\n"
    "jumper-find-file() {\n"
    "    selected=$(__jumper_ffile)\n"
    "    pre=\"${READLINE_LINE:0:$READLINE_POINT}\"\n"
    "    READLINE_LINE=\"${pre}$selected${READLINE_LINE:$READLINE_POINT}\"\n"
    "    READLINE_POINT=$(( READLINE_POINT + ${#selected} ))\n"
    "}\n";

static const char bash_bindings[] =
    "bind -x '\"\\C-y\": jumper-find-dir'\n"
    "stty kill undef\n"
    "bind -x '\"\\C-u\": jumper-find-file'\n"
    "PROMPT_COMMAND=\"__jumper_update_db;$PROMPT_COMMAND\"\n";

static const char zsh_variables[] =
    "[[ -n $__JUMPER_FLAGS ]] || __JUMPER_FLAGS='-cHo -n 500'\n"
    "[[ -n $__JUMPER_FZF_OPTS ]] || __JUMPER_FZF_OPTS=(--height=70%% "
    "--layout=reverse --keep-right --preview-window=hidden --ansi)\n"
    "if [[ -z $__JUMPER_FZF_FILES_PREVIEW ]]; then\n"
    "    if [[ -n $(which bat) ]]; then\n"
    "        __JUMPER_FZF_FILES_PREVIEW='bat --color=always'\n"
    "    else\n"
    "        __JUMPER_FZF_FILES_PREVIEW='cat'\n"
    "    fi\n"
    "fi\n"
    "[[ -n $__JUMPER_FZF_FOLDERS_PREVIEW ]] || "
    "__JUMPER_FZF_FOLDERS_PREVIEW='ls -1UpC --color=always'\n"
    "[[ -n $__JUMPER_TOGGLE_PREVIEW ]] || __JUMPER_TOGGLE_PREVIEW='ctrl-p'\n";

static const char zsh_functions[] =
    "z() {\n"
    "    args=\"${@// /\\ }\"\n"
    "	new_path=$(jumper find --type=directories -n 1 \"${args}\")\n"
    "	if [[ -z $new_path ]]; then\n"
    "		echo 'No match found.'\n"
    "	else\n"
    "		cd \"${new_path}\"\n"
    "	fi\n"
    "}\n"
    "zf() {\n"
    "    args=\"${@// /\\ }\"\n"
    "    file=$(jumper find --type=files -n 1 \"${args}\")\n"
    "	if [[ -z $file ]]; then\n"
    "		echo 'No match found.'\n"
    "    else\n"
    "		eval \"${EDITOR} '${file}'\"\n"
    "	fi\n"
    "}\n"
    "__jumper_fdir() {\n"
    "	__JUMPER=\"jumper find --type=directories ${__JUMPER_FLAGS}\"\n"
    "	fzf ${__JUMPER_FZF_OPTS} --disabled --query \"$1\" \\\n"
    "        --preview \"eval x={}; ${__JUMPER_FZF_FOLDERS_PREVIEW} \\$x\" \\\n"
    "        --bind \"${__JUMPER_TOGGLE_PREVIEW}:toggle-preview\" \\\n"
    "		--bind \"start:reload:${__JUMPER} {q}\" \\\n"
    "		--bind \"change:reload:sleep 0.05; ${__JUMPER} {q} || true\"\n"
    "}\n"
    "__jumper_ffile() {\n"
    "	__JUMPER=\"jumper find --type=files ${__JUMPER_FLAGS}\"\n"
    "	fzf $__JUMPER_FZF_OPTS --disabled --query \"$1\" \\\n"
    "        --preview \"eval x={}; ${__JUMPER_FZF_FILES_PREVIEW} \\$x\" \\\n"
    "        --bind \"${__JUMPER_TOGGLE_PREVIEW}:toggle-preview\" \\\n"
    "		--bind \"start:reload:${__JUMPER} {q}\" \\\n"
    "		--bind \"change:reload:sleep 0.05; ${__JUMPER} {q} || true\"\n"
    "}\n"
    "zi() {\n"
    "	new_path=$(__jumper_fdir)\n"
    "	if [[ -n $new_path ]]; then\n"
    "        # Manually perform tilde expansion\n"
    "		cd \"${new_path/#\\~/$HOME}\"\n"
    "	fi\n"
    "}\n"
    "zfi() {\n"
    "    file=$(__jumper_ffile)\n"
    "	if [[ -n $file ]]; then\n"
    "        # Manually perform tilde expansion\n"
    "		$EDITOR \"${file/#\\~/$HOME}\"\n"
    "	fi\n"
    "}\n"
    "__jumper_delete_duplicate_symlinks(){\n"
    "    tempfile=\"${__JUMPER_FOLDERS}_temp\"\n"
    "    cp \"${__JUMPER_FOLDERS}\" \"${tempfile}\"\n"
    "    while IFS= read -r line ; do\n"
    "        entry=\"${line%%%%|*}\"\n"
    "        real=$(realpath $entry)\n"
    "        if [[ $entry != $real ]]; then\n"
    "            sed -i -e \"\\:^${real}|:d\" \"${tempfile}\"\n"
    "        fi\n"
    "    done < \"${__JUMPER_FOLDERS}\"\n"
    "    mv \"${tempfile}\" \"${__JUMPER_FOLDERS}\"\n"
    "}\n"
    "__jumper_replace_symlinks(){\n"
    "    tempfile=\"${__JUMPER_FOLDERS}_temp\"\n"
    "    cp \"${__JUMPER_FOLDERS}\" \"${tempfile}\"\n"
    "    while IFS= read -r line ; do\n"
    "        entry=\"${line%%%%|*}\"\n"
    "        real=$(realpath $entry)\n"
    "        if [[ $entry != $real ]]; then\n"
    "            sed -i -e \"s:^${real}/:${entry}/:g\" \"${tempfile}\"\n"
    "        fi\n"
    "    done < \"${__JUMPER_FOLDERS}\"\n"
    "    mv \"${tempfile}\" \"${__JUMPER_FOLDERS}\"\n"
    "}\n"
    "__jumper_clean_symlinks(){\n"
    "    __jumper_delete_duplicate_symlinks\n"
    "    __jumper_replace_symlinks\n"
    "}\n"
    "_jumper_clean() {\n"
    "    (jumper clean --type=files > /dev/null 2>&1 && jumper clean "
    "--type=directories > /dev/null 2>&1 &)\n"
    "}\n"
    "__jumper_update_db() {\n"
    "    if [[ ! -z $__jumper_current_folder ]]; then\n"
    "        if [[ $__jumper_current_folder != $PWD ]]; then\n"
    "            # working directory has changed, this visit has more weight\n"
    "            jumper update --type=directories -w 1.0 \"$PWD\"\n"
    "        else\n"
    "            # working directory has not changed\n"
    "            jumper update --type=directories -w 0.3 \"$PWD\"\n"
    "        fi\n"
    "    fi\n"
    "    __jumper_current_folder=$PWD\n"
    "    # Remove files and folders that do not exist anymore\n"
    "    if [[ -n $__JUMPER_CLEAN_FREQ ]] && [[ $(( RANDOM %% "
    "__JUMPER_CLEAN_FREQ )) == 0 ]]; then\n"
    "        _jumper_clean\n"
    "    fi\n"
    "}\n"
    "jumper-find-dir() {\n"
    "    selected=$(__jumper_fdir)\n"
    "    LBUFFER=\"${LBUFFER}${selected}\"\n"
    "    zle reset-prompt\n"
    "}\n"
    "jumper-find-file() {\n"
    "    selected=$(__jumper_ffile)\n"
    "    LBUFFER=\"${LBUFFER}${selected}\"\n"
    "    zle reset-prompt\n"
    "}\n";

static const char zsh_bindings[] = "zle -N jumper-find-dir\n"
                                   "zle -N jumper-find-file\n"
                                   "bindkey '^Y' jumper-find-dir\n"
                                   "bindkey '^U' jumper-find-file\n"
                                   "precmd() {\n"
                                   "    __jumper_update_db\n"
                                   "}\n";

static const char fish_variables[] =
    "if not set -q __JUMPER_FLAGS\n"
    "    set __JUMPER_FLAGS '-cHo -n 500'\n"
    "end\n"
    "if not set -q __JUMPER_FZF_OPTS\n"
    "    set __JUMPER_FZF_OPTS --height=70%% --layout=reverse --keep-right "
    "--preview-window=hidden --ansi\n"
    "end\n"
    "if not set -q __JUMPER_FZF_FILES_PREVIEW\n"
    "    if type -q bat\n"
    "        set __JUMPER_FZF_FILES_PREVIEW 'bat --color=always'\n"
    "    else\n"
    "        set __JUMPER_FZF_FILES_PREVIEW 'cat'\n"
    "    end\n"
    "end\n"
    "if not set -q __JUMPER_FZF_FOLDERS_PREVIEW\n"
    "    set __JUMPER_FZF_FOLDERS_PREVIEW 'ls -1UpC --color=always'\n"
    "end\n"
    "if not set -q __JUMPER_TOGGLE_PREVIEW\n"
    "    set __JUMPER_TOGGLE_PREVIEW 'ctrl-p'\n"
    "end\n";

static const char fish_functions[] =
    "function __jumper_clean -d \"clean jumper's files' and folders' "
    "databases\"\n"
    "    jumper clean --type=files > /dev/null 2>&1 && jumper clean "
    "--type=directories> /dev/null 2>&1 &\n"
    "end\n"
    "function jumper_update_db --on-event fish_postexec\n"
    "    if set -q __jumper_current_folder\n"
    "        if [ $__jumper_current_folder != $PWD ]\n"
    "            # working directory has changed, this visit has more weight\n"
    "            jumper update --type=directories -w 1.0 \"$PWD\"\n"
    "        else\n"
    "            # working directory has not changed\n"
    "            jumper update --type=directories -w 0.3 \"$PWD\"\n"
    "        end\n"
    "    end\n"
    "    set -g __jumper_current_folder \"$PWD\"\n"
    "    if [ -n \"$__JUMPER_CLEAN_FREQ\" ] && [ (math (random) %% "
    "$__JUMPER_CLEAN_FREQ) -eq 0 ]\n"
    "        __jumper_clean\n"
    "    end\n"
    "end\n"
    "function z -d \"Jump to folder\"\n"
    "    set -l new_path (jumper find --type=directories -n 1 \"$argv\")\n"
    "    if [ -n \"$new_path\" ]\n"
    "        cd \"$new_path\"\n"
    "    else\n"
    "        echo 'No match found'\n"
    "    end\n"
    "end\n"
    "function zf -d \"Jump to file\"\n"
    "    set -l file (jumper find --type=files -n 1 \"$argv\")\n"
    "    if [ -n \"$file\" ]\n"
    "        eval \"$EDITOR '$file'\"\n"
    "    else\n"
    "        echo 'No match found'\n"
    "    end\n"
    "end\n"
    "function __jumper_fdir -d \"run fzf on jumper's directories\"\n"
    "    set -l __JUMPER \"jumper find --type=directories $__JUMPER_FLAGS\"\n"
    "    fzf $__JUMPER_FZF_OPTS --disabled --query \"$argv\" \\\n"
    "        --preview \"eval x={}; $__JUMPER_FZF_FOLDERS_PREVIEW \\$x\" \\\n"
    "        --bind \"$__JUMPER_TOGGLE_PREVIEW:toggle-preview\" \\\n"
    "        --bind \"start:reload:$__JUMPER {q}\" \\\n"
    "        --bind \"change:reload:sleep 0.05; $__JUMPER {q} || true\"\n"
    "end\n"
    "function __jumper_ffile -d \"run fzf on jumper's files\"\n"
    "    set -l __JUMPER \"jumper find --type=files $__JUMPER_FLAGS\"\n"
    "    fzf $__JUMPER_FZF_OPTS --disabled --query \"$argv\" \\\n"
    "        --preview \"eval x={}; $__JUMPER_FZF_FILES_PREVIEW \\$x\" \\\n"
    "        --bind \"$__JUMPER_TOGGLE_PREVIEW:toggle-preview\" \\\n"
    "        --bind \"start:reload:$__JUMPER {q}\" \\\n"
    "        --bind \"change:reload:sleep 0.05; $__JUMPER {q} || true\"\n"
    "end\n"
    "function zi -d \"Interactive jump to folder\"\n"
    "	set new_path (__jumper_fdir)\n"
    "	if [ -n \"$new_path\" ]\n"
    "        set new_path (string replace '~' $HOME $new_path)\n"
    "		cd \"$new_path\"\n"
    "	end\n"
    "end\n"
    "function zfi -d \"Interactive jump to file\"\n"
    "    set file (__jumper_ffile)\n"
    "	if [ -n \"$file\" ]\n"
    "        set file (string replace '~' $HOME $file)\n"
    "		eval \"$EDITOR '$file'\"\n"
    "	end\n"
    "end\n"
    "function jumper-find-dir -d \"Fuzzy-find directories\"\n"
    "    set -l commandline (commandline -t)\n"
    "    set result (__jumper_fdir)\n"
    "    if [ -n \"$result\" ]\n"
    "        commandline -it -- $result\n"
    "    end\n"
    "    commandline -f repaint\n"
    "end\n"
    "function jumper-find-file -d \"Fuzzy-find files\"\n"
    "    set -l commandline (commandline -t)\n"
    "    set result (__jumper_ffile)\n"
    "    if [ -n \"$result\" ]\n"
    "        commandline -it -- $result\n"
    "    end\n"
    "    commandline -f repaint\n"
    "end\n";

static const char fish_bindings[] = "bind \\cy jumper-find-dir\n"
                                    "bind \\cu jumper-find-file\n";

void shell_setup(const char *shell) {
  if (strcmp(shell, "bash") == 0) {
    printf(bash_variables);
    printf(bash_functions);
    printf(bash_bindings);
  } else if (strcmp(shell, "zsh") == 0) {
    printf(zsh_variables);
    printf(zsh_functions);
    printf(zsh_bindings);
  } else if (strcmp(shell, "fish") == 0) {
    printf(fish_variables);
    printf(fish_functions);
    printf(fish_bindings);
  } else {
    fprintf(stderr, "ERROR: Invalid argument for shell: %s.\n", shell);
    fprintf(stderr, "Accepted arguments: bash, zsh, fish.\n");
    exit(EXIT_FAILURE);
  }
}