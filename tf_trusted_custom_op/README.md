## TF Trusted Custom Operation

These instructions guide you through building the TF Trusted custom operation that is required to run models directly inside of an Intel SGX device.

### MacOS Build

#### Build Custom Operation

Run the following commands to build the custom operation.

```
$ ./configure.sh
$ bazel build model_enclave_op.so
$ cp bazel-bin/model_enclave_op.so .
```

### Linux Build

#### Start Container

For this command to work correctly we need to be in the root directory of this repo.

```
$ cd ..
$ sudo docker run -it -v `pwd`:/opt/my-project \
  -w /opt/my-project/tf_trusted_custom_op \
  tensorflow/tensorflow:custom-op /bin/bash
```

#### Build Custom Operation

Run the following commands to build the custom operation.

```
$ ./configure.sh
$ bazel build model_enclave_op.so
$ cp bazel-bin/model_enclave_op.so .
```

We can now exit out of the docker container.

```
exit
```
