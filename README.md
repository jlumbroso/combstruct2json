# combstruct2json

Lightweight library to parse
[`combstruct` grammars](https://www.maplesoft.com/support/help/maple/view.aspx?path=combstruct),
and standalone tool to convert them to JSON.

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
$ make
$ ./combstruct tests/cographs
{ "G": { "op": "Set", "param": [{ "id": "Co" }] }, "Co": { "op": "Union", "param": [ { "id": "Ge" }, { "id": "Gc" }, { "id": "v" }, { "op": "Prod", "param": [ { "id": "v" }, { "id": "v" } ] } ] }, "Ge": { "op": "Union", "param": [ { "op": "Set", "param": [{ "id": "Sc" }], "restriction": "card = 2" }, { "op": "Prod", "param": [ { "id": "Sc" }, { "id": "v" } ] } ] }, "Gc": { "op": "Set", "param": [{ "op": "Union", "param": [ { "id": "v" }, { "id": "Sc" } ] }], "restriction": "card >= 3" }, "Sc": { "op": "Set", "param": [{ "op": "Union", "param": [ { "id": "v" }, { "id": "C" } ] }], "restriction": "card >= 2" }, "C": { "op": "Set", "param": [{ "op": "Union", "param": [ { "id": "v" }, { "id": "Sc" } ] }], "restriction": "card >= 2" }, "v": { "type": "unit", "unit": "Atom" }}
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
                        "id": "v"
                    },
                    ...
```

## Acknowledgements

Thanks to Alexandre de Faveri.

## Bibliography

Philippe Flajolet, Paul Zimmermann and  Bernard van Cutsem. [*A calculus for the random generation of combinatorial structures.*](http://algo.inria.fr/flajolet/Publications/RR-1830.pdf) 29 pages. Theoretical Computer Science, vol. 132 (1-2), pp. 1-35, 1994.

Paul Zimmermann. [*Gaia: a package for the random generation of combinatorial structures.*](http://plouffe.fr/simon/math/fpsac1993b.pdf#page=153) Maple Technical Newsletter, vol. 1 (1), pp. 143-147, 1994.

