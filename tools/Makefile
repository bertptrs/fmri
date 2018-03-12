MKDIR=mkdir -p

all: generated

generated: $(wildcard proto/*.proto)
	$(MKDIR) $@
	protoc --python_out="$@" $^
