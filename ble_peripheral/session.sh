if [[ $# -eq 0 ]] ; then
	vim -S ~/.vim/sessions/balance.vim
    exit 0
fi

while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    le)
		vim -S ~/.vim/sessions/zulp.vim
        ;;
    *)
        ;;
esac
shift # past argument or value
done

