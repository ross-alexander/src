package main

import "io/ioutil"
import "fmt"
import "log"
import "path/filepath"
import "os"

func scandir(dir string) {
	files, err := ioutil.ReadDir(dir)
	if (err != nil) {
		log.Fatal(err)
	}
	for _, file := range files {
		path := filepath.Join(dir, file.Name())

		if match, err := filepath.Match("*.m3u", file.Name()); err == nil && match {
			fmt.Println(path)
		}
		if stat, err := os.Stat(path); err == nil && stat.IsDir() {
			scandir(path)
		}
	}
}

func main() {
	scandir("/locker/flac")
}
