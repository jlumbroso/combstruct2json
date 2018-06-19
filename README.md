# combstruct2json

This project provides:
1. A highly optimized, extensible, lightweight linkable library to parse
[`combstruct` grammars](https://www.maplesoft.com/support/help/maple/view.aspx?path=combstruct),
implemented in C/C++.
2. An independently available Python wrapper that, given a grammar file, outputs a `dict` according to the JSON specification below.
3. A standalone commandline utility that can be piped to other tools (or called as an external system command from a host language such a Ruby, or JavaScript).

The folder `examples` contains usage examples for the linkable library, the folder `tests` contains sample grammars that can be parsed.

This library is already used by projects, such as [boltzoc](https://github.com/jlumbroso/boltzoc), the fast Analytic Sampler Oracle in C.

## Example

```bash
$ cat tests/cographs
G  = Set(Co),
Co = Union(Ge, Gc, v, Prod(v,v)),
Ge = Union(Set(Sc, card=2), Prod(Sc,v)),
Gc = Set(Union(v, Sc), card>=3),
Sc = Set(Union(v, C), card>=2),
C  = Set(Union(v, Sc), card>=2),
v  = Atom
```

would then produce the following JSON output:

```bash
$ make all
$ ./combstruct2json tests/cographs
{ "G": { "type": "op", "op": "Set", "param": [{ "type": "id", "id": "Co" }] }, "Co": { "type": "op", "op": "Union", "param": [ { "type": "id", "id": "Ge" }, { "type": "id", "id":"Gc" }, { "type": "id", "id": "v" }, { "type": "op", "op": "Prod", "param": [ { "type": "id", "id": "v" }, { "type": "id", "id": "v" } ] } ] }, "Ge": { "type": "op", "op": "Union", "param": [ { "type": "op", "op": "Set", "param": [{ "type": "id", "id": "Sc" }], "restriction": "card = 2" }, { "type": "op", "op": "Prod", "param": [ { "type": "id", "id": "Sc" }, { "type": "id", "id": "v" } ] } ] }, "Gc": { "type": "op", "op": "Set", "param": [{ "type": "op", "op": "Union", "param": [ { "type": "id", "id": "v" }, { "type": "id", "id":"Sc" } ] }], "restriction": "card >= 3" }, "Sc": { "type": "op", "op": "Set", "param": [{ "type": "op", "op": "Union", "param": [ { "type": "id", "id": "v" }, { "type": "id", "id": "C" } ] }], "restriction": "card >= 2" }, "C": { "type": "op", "op": "Set", "param": [{ "type": "op", "op": "Union", "param": [ { "type": "id", "id": "v" }, { "type": "id", "id": "Sc" } ] }], "restriction": "card >= 2" }, "v": { "type": "unit", "unit": "Atom" }}
```

which can be prettified, for instance, using Python, for better legibility:

```bash
$ ./combstruct2json tests/cographs | python -m json.tool | head
{
    "C": {
        "op": "Set",
        "param": [
            {
                "op": "Union",
                "param": [
                    {
                        "id": "v",
                        "type": "id"
                    ...
```

If you build and install the Python wrapper, you may also read a grammar directly
from a Python program:

```python
import combstruct2json
d = combstruct2json.read_file("tests/cographs")
print("Top-level symbols:")
print(d.keys())
```

which would print out:

```text
Top-level symbols:
[u'C', u'Co', u'G', u'Ge', u'Gc', u'v', u'Sc']
```

## Installation

You can build the project from scratch, if you have the necessary dependencies:

1. You may need to install `flex` and `bison`, if you don't already have them.

2. Run `make all` to create the executable `combstruct2json`, the static C/C++
   library, and the Python wrapper library.

3. Run `./combstruct2json <filename>` to print the parsed JSON output, from the
   grammar contained in the given file.

Alternatively, you may also install the Python library directly from PyPI
(possibly in your user directory). This step will fetch the latest source after
it has been processed by lexer/parser and so does not require they be installed:

```bash
$ pip install combstruct2json
```

## Draft specification of JSON output

Because one purpose to enable easier interoperability with existing work using
symbolic specifications in Maple, this library uses
[Maple's specification](https://www.maplesoft.com/support/help/maple/view.aspx?path=combstruct)
for `combstruct` grammars as a starting point.

The output is a JSON string which represents a dictionary mapping symbol names
to an abstract syntax tree. Each node of the grammar is represented by a node
in the JSON tree:

- For unit elements (with or without a weight), the `type` is `unit`; the
  available field is `unit` to describe the type of element (an atom, or epsilon). Example:

  ```
  { "type": "unit", "unit": "Epsilon" }
  ```

- For variable references, the `type` is `id`; the available field is `id` which
  should specify the name of the variable that is being referenced.
  Example:

  ```
  { "type": "id", "id": "v" }
  ```

- For operators, the `type` is `op`; the available fields are `op` which describes
  which operator is applied (from `Union`, `Prod`, `Sequence`, `Set`, etc.),
  `param` which would be a list of parameters on which the operator is applied, and `restriction` which optionally encodes a restriction (for the moment, this is
  limited to cardinality restrictions). Example:

  ```
  {
    "type": "op",
    "op": "Union",
    "params": [
        { "type": "id", "id": "v" },
        { "type": "id", "id": "Sc" }
        ],
    "restriction": "card >= 2"
  }
  ```

This draft specification is designed to produce an easy to parse JSON grammar format
specification to improve communication between various tools of a planned analytic
combinatorics toolchain.

## Benchmark

Although we do not currently have benchmarks, this parser library and tool were
designed using the most low-level tools, so as to be able to parse the possibly
very large grammars that may be the output of algorithms. Such an example,
[reluctant random walks](https://github.com/jlumbroso/reluctant-walks), provides
one very large example `reluctantQPW2` that has more than 1000 equations (these
are lines that are not comments) and weighs 4.6 MB:

```bash
$ awk 'flag { print } /^$/ { flag = 1 }' tests/reluctantQPW2 | wc -l
    1066

$ time ./combstruct2json tests/reluctantQPW2  > /dev/null
real	0m3.577s
user	0m3.459s
sys	0m0.099s
```

Another grammar for the same walk model, generated by the same process but with
more accuracy, had 5000+ equations and a specification weighing 115.95 MB (it
could not be included in this repository because it exceeds GitHub's limits):

```bash
$ awk 'flag { print } /^$/ { flag = 1 }' tests/reluctantQPW2_ | wc -l
    5093

$ time ./combstruct2json tests/reluctantQPW2_  > /dev/null
real	6m7.458s
user	5m53.586s
sys	0m9.186s
```

## Acknowledgements

Thanks to Alexandre de Faveri.

## Bibliography

Philippe Flajolet, Paul Zimmermann and  Bernard van Cutsem. [*A calculus for the random generation of combinatorial structures.*](http://algo.inria.fr/flajolet/Publications/RR-1830.pdf) 29 pages. Theoretical Computer Science, vol. 132 (1-2), pp. 1-35, 1994.

Paul Zimmermann. [*Gaia: a package for the random generation of combinatorial structures.*](http://plouffe.fr/simon/math/fpsac1993b.pdf#page=153) Maple Technical Newsletter, vol. 1 (1), pp. 143-147, 1994.

