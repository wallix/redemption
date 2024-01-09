* Build image::

    docker build -t redemption-build -f Dockerfile .


* Execute::

    docker run -ti --rm -v $(pwd):/$(basename $(pwd)) -w /$(basename $(pwd)) -u $(id -u ${USER}):$(id -g ${USER}) redemption-build
