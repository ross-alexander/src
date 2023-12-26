.suffix: go

ALL: portscan portscan.exe

portscan-w64: portscan.go
	GOOS=windows GOARCH=amd64 go build $<

portscan : portscan.go
	go build $<

test : test.go
	/opt/go/bin/go build $<
