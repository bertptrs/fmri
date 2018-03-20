#!/bin/bash

# Make sure we're in the right directory
cd "$(dirname "${BASH_SOURCE[0]}")" || (echo "Could not change dir" && exit 1)
pwd

for modeldir in models/*; do
	if ! pushd "$modeldir" &>/dev/null; then
		continue
	fi

	wget --continue -i weights_url -N
	popd || exit 1
done

