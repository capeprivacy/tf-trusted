import tensorflow as tf
import os
import argparse
from tensorflow import gfile
import numpy as np
from tensorflow.python.client import timeline
import tf_trusted_custom_op as tft

parser = argparse.ArgumentParser()
parser.add_argument(
    '--model_file', type=str, default='', help='Frozen GraphDef model file location')
parser.add_argument(
    '--input_file', type=str, default="", help='Load the data from a numpy file format (.npy)')
parser.add_argument('--input_name', type=str, default='', help='Name of the input Placeholder node')
parser.add_argument('--output_name', type=str, default='', help='Name of the output node')
parser.add_argument('--benchmark', action='store_true', help='Run 100 timed inferences, results are stored in /tmp/tensorboard')
parser.add_argument('--batch_size', type=int, default='1', help='Batch size must match first dim of input file')
parser.add_argument('--model_name', type=str, default='model', help='Name your model!')
parser.add_argument('--input_shape', nargs='+', help='The input shape')
parser.add_argument('--from_file', action='store_true',
                    help='Tell the enclave to read from a file, file must exists on the enclave machine and already converted to tflite format')
config = parser.parse_args()

dirname = os.path.dirname(tft.__file__)
shared_object = dirname + '/model_enclave_op.so'

model_module = tf.load_op_library(shared_object)
model_load = model_module.model_load_enclave
model_predict = model_module.model_predict_enclave

model_file = config.model_file
input_file = config.input_file
input_name = config.input_name
output_name = config.output_name
benchmark = config.benchmark
batch_size = config.batch_size
model_name = config.model_name
from_file = config.from_file


def get_output_shape_and_type(model_file, output_name):
    with gfile.GFile(model_file, 'rb') as f:
        graph_def = tf.GraphDef()
        graph_def.ParseFromString(f.read())

    tf.graph_util.import_graph_def(graph_def)

    # prepend import name gets added when calling import_graph_def
    output = tf.get_default_session().graph.get_tensor_by_name('import/' + output_name + ":0")
    shape = list(output.get_shape())

    # TODO i think this can just be inferred via the custom op
    shape[0] = batch_size

    return shape, output.dtype


def get_input_shape(model_file, input_name):
    with gfile.GFile(model_file, 'rb') as f:
        graph_def = tf.GraphDef()
        graph_def.ParseFromString(f.read())

    tf.graph_util.import_graph_def(graph_def)

    # prepend import name gets added when calling import_graph_def
    input = tf.get_default_session().graph.get_tensor_by_name('import/' + input_name + ":0")

    if config.input_shape is not None:
        shape = config.input_shape
    else:
        try:
            shape = list(input.get_shape())
        except ValueError:
            print("Error: Can't read shape from input try setting --input_shape instead")
            exit()

    # TODO i think this can just be inferred via the custom op
    shape[0] = batch_size

    return shape


def convert_model_to_tflite(graph_def_file, input_arrays, output_arrays, input_shape):
    converter = tf.lite.TFLiteConverter.from_frozen_graph(
        graph_def_file, input_arrays, output_arrays, input_shapes={input_arrays[0]: input_shape})
    tflite_model = converter.convert()

    return bytes(tflite_model)


def save_to_tensorboard(i, sess, run_metadata):
    writer = tf.summary.FileWriter("/tmp/tensorboard/run" + str(i), sess.graph)

    session_tag = "prediction" + str(i)
    writer.add_run_metadata(run_metadata, session_tag)
    writer.close()

    tracer = timeline.Timeline(run_metadata.step_stats)
    chrome_trace = tracer.generate_chrome_trace_format()
    with open('{}/{}.ctr'.format("/tmp/tensorboard", session_tag), 'w') as f:
        f.write(chrome_trace)


with tf.Session() as sess:
    input_shape = get_input_shape(model_file, input_name)
    output_shape, output_type = get_output_shape_and_type(model_file, output_name)


tflite_bytes = []
if not from_file:
    tflite_bytes = convert_model_to_tflite(model_file, [input_name], [output_name], input_shape)

put = np.load(input_file)

if benchmark:
    tf.reset_default_graph()

    with tf.Session() as sess:
        load_node = model_load(model_name, tflite_bytes)
        load_node.run()

        for i in range(0, 100 + 1):
            placeholder = tf.placeholder(shape=input_shape, dtype=tf.float32)
            out = model_predict(model_name, placeholder, output_shape, dtype=output_type)

            meta = tf.RunMetadata()
            sess.run(out, feed_dict={placeholder: put}, options=tf.RunOptions(trace_level=tf.RunOptions.FULL_TRACE),
                     run_metadata=meta)
            save_to_tensorboard(i, sess, meta)
else:
    with tf.Session() as sess:
        load_node = model_load(model_name, tflite_bytes)
        load_node.run()

        placeholder = tf.placeholder(shape=input_shape, dtype=tf.float32)
        out = model_predict(model_name, placeholder, output_shape, dtype=output_type)
        actual = sess.run(out, feed_dict={placeholder: put})
        print("Prediction: ", actual)
