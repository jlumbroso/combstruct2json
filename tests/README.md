
# Description of tests

## Simple tests

This folder contains very simple test cases meant to check whether the error reporting system is working properly:

- `test1` and `test3` should parse without errors;
- `test2` should have lexer and parser errors;
- `test4` should have only lexer errors.

## Extended tests

We also have certain standard specification to use as examples:

- `cographs` is a symbolic grammar describing cographs, following the split decomposition framework of Bahrani and Lumbroso (2017);
- `umlmodel` is the demonstration grammar used in the paper by Mougenot, Darrasse, Blanc and Soria (2009);
- `reluctantQPW1` is the automatically generated half-plane walk grammar that is optimal for the random sampling of a reluctant quarter-plane walk, a walk of which the drift is very negative, following the work of Lumbroso, Mishna, Ponty (2016).

Finally, we have a few specifications from the Encyclopedia of Combinatorial Structures (ECS), in the folder `ecs`.