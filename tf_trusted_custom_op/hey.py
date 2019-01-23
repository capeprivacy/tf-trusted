import tensorflow as tf
from tensorflow.python.client import timeline
import numpy as np


with tf.Session() as sess:
    with tf.name_scope("matmul"):
        put = tf.placeholder(shape=[10, 10], dtype=tf.float64)
        out = tf.matmul(put, np.random.uniform(size=[10, 100]))

    session_tag = "session" + str(0)
    writer = tf.summary.FileWriter("/tmp/tensorboard2/" + session_tag, sess.graph)

    run_metadata = tf.RunMetadata()
    run_options = tf.RunOptions(trace_level=tf.RunOptions.FULL_TRACE)
    sess.run(out, feed_dict={put: np.random.uniform(size=[10, 10])}, options=run_options,
             run_metadata=run_metadata)

    writer.add_run_metadata(run_metadata, session_tag)
    # writer.close()

    tracer = timeline.Timeline(run_metadata.step_stats)
    chrome_trace = tracer.generate_chrome_trace_format()
    with open('{}/{}.ctr'.format("/tmp/tensorboard2", session_tag), 'w') as f:
        f.write(chrome_trace)
