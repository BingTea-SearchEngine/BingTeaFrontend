# BingTeaFrontend
A frontend server that clients can type queries in and get search results

## Setting up Index

Download the zipped index here
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
