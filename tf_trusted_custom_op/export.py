import tensorflow as tf
import pandas as pd

from tensorflow.python.framework import graph_io
from tensorflow.python.framework import graph_util

from tensorflow import keras
from tensorflow.keras import layers
from tensorflow.keras import backend as K
import argparse
from tensorflow.python import errors_impl as errors
import os
import time
import numpy as np


def export_to_pb(sess, x, filename):
    pred_names = ['output']
    tf.identity(x, name=pred_names[0])

    graph = graph_util.convert_variables_to_constants(sess, sess.graph.as_graph_def(), pred_names)

    graph = graph_util.remove_training_nodes(graph)
    path = graph_io.write_graph(graph, ".", filename, as_text=False)
    print('saved the frozen graph (ready for inference) at: ', path)


with tf.Session() as sess:
    a = tf.placeholder(shape=[1, 528], dtype=tf.float32)

    w = np.ones([528, 1]).astype(np.float32)
    w.fill(0.5)

    x = tf.matmul(a, w)
    x = tf.nn.sigmoid(x)

    export_to_pb(sess, x, "random.pb")

    print(sess.run(x, feed_dict={a: np.load("input.npy")}))
