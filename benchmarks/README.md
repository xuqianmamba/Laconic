Datasets
======================
We have stored a sample dataset named uk-2007-05@1000000 in this directory, which contains several folders representing data at different stages of compression.

## Origin
The origin graph in txt and binary(npy) format.

## Laconic_rule_compressed
This stage demonstrates the state of the data after it has been compressed using Laconic encoding.

## Laconic_encoding_compressed
The directory contains the results of compression using Laconic encoding, meaning it holds the data after Laconic compression has been applied.

## Reorder
This directory contains the size of the graph after Laconic has applied rule-based compression, followed by reordering and then encoding compression. Users can compare the graph size in this directory with that in the 'Laconic_encoding_compressed' directory to assess the improvement in compression ratio brought about by the reordering.

## Peak_memory
This directory showcases real-time memory monitoring for each module during the Laconic compression process. In particular, the rule-based compression part is divided into batches of different sizes to demonstrate the impact of the number of batches on peak memory usage.
