while inotifywait -r -e modify,create,delete "../"; do
  echo "Reloaded"
  make
done
