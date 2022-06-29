while [ 1 ]; do
	inotifywait --event modify ./cont.md
	pandoc cont.md > cont.html
	echo "updated file"
done
