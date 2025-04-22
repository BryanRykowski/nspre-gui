Included Dockerfile builds the project in Debian Bookworm


Build docker image (run in same directory as Dockerfile)
```
docker build -t IMAGE_NAME .
```
Run image (pulls latest commit to main and builds in /build)
```
docker run -v BUILD_DESTINATION:/build:Z --rm -it NAME
```
