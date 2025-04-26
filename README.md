# BingTeaFrontend
A frontend server that clients can type queries in and get search results

## Setting up Index

Download the zipped index [here](https://drive.google.com/file/d/1qlDnK5gh8ler_f30OEn1ZsFBuPLp45_K/view?usp=sharing)
Run the indexing program
```
# Clone index repo
git clone https://github.com/BingTea-SearchEngine/Reindex.git
git switch wonbin-dev
mkdir build
cd build
cmake ..
make -j

# Run the indexing program
./reindex -i <path to parsed html files> -o <desired output path> -s <chunk size in MB>

# Start the index server
./IndexServer -i <path to indexing output> -h <path to parsed html files> -t <upper bound on search time> -n <number of chunks in memory> -c <max number of matches to look for>
```

## Running frontend
Go to serverips.txt in project root and add 127.0.0.1:8002

```
# Compile
mkdir build
cd build
cmake ..
make -j

# Run the frontend
./BingTeaFrontend
```

Go to localhost:8002 on browser
