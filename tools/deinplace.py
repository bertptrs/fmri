#!/usr/bin/env python3

import argparse
import sys
import generated.proto.caffe_pb2 as pb2
import google.protobuf.text_format as text_format


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def get_args():
    parser = argparse.ArgumentParser(description='De-in-place Caffe networks')
    parser.add_argument('file', type=argparse.FileType('r'))
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Be more verbose.')
    parser.add_argument('-o', '--output', type=argparse.FileType('w'),
                        default=sys.stdout,
                        help='File to write result to. Default stdout')

    return parser.parse_args()


def load_net(args):
    with args.file as f:
        data = "".join(f.readlines())
        net = pb2.NetParameter()
        text_format.Merge(data, net)

        return net


def deinplace(args, net):
    outputs = {}
    for layer in net.layer:
        for idx, bottom in enumerate(layer.bottom):
            if bottom in outputs and bottom != outputs[bottom]:
                if args.verbose:
                    eprint(layer.name, 'reads from in-place layer, rewriting…')

                layer.bottom[idx] = outputs[bottom]

        for idx, top in enumerate(layer.top):
            if top in outputs:
                if args.verbose:
                    eprint(layer.name, 'works in-place, rewriting…')
                outputs[top] = layer.name
                layer.top[idx] = layer.name
            else:
                outputs[top] = top


def main():
    args = get_args()
    net = load_net(args)
    deinplace(args, net)

    args.output.write(text_format.MessageToString(net))


if __name__ == '__main__':
    main()
