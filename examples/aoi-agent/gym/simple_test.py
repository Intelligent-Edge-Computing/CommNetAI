#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gymnasium as gym
import argparse
import ns3gym
from ns3gym import ns3env
import random
import string
from stable_baselines3 import DQN

__author__ = "Jianpeng Qi"
__copyright__ = "Copyright (c) 2024, Ocean University of China"
__version__ = "0.1.0"
__email__ = "qijianpeng@ouc.edu.cn"

def generate_random_suffix(length=5):
    # 生成一个长度为 `length` 的随机后缀
    return ''.join(random.choices(string.ascii_lowercase + string.digits, k=length))
# 使用生成的后缀
suffix = generate_random_suffix()
model_save_name = f"dqn_caoi_{suffix}"

env = gym.make('ns3-v0')
model = DQN("MlpPolicy", env, verbose=1)
model.learn(total_timesteps=50, log_interval=4)
model.save(model_save_name)
obs, info = env.reset(seed=99)
try:

    while True:
        action, _states = model.predict(obs)

        obs, rewards, terminated, truncated, info = env.step(int(action))
        if terminated or truncated:
            break
#
except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")

# env.reset()
#
# ob_space = env.observation_space
# ac_space = env.action_space
# print("Observation space: ", ob_space,  ob_space.dtype)
# print("Action space: ", ac_space, ac_space.dtype)
#
# stepIdx = 0
#
# try:
#     obs = env.reset()
#     print("Step: ", stepIdx)
#     print("---obs: ", obs)
#
#     while True:
#         stepIdx += 1
#
#         action = env.action_space.sample()
#         print("---action: ", action)
#         obs, reward, done, info = env.step(action)
#
#         print("Step: ", stepIdx)
#         print("---obs, reward, done, info: ", obs, reward, done, info)
#
#         if done:
#             break
#
# except KeyboardInterrupt:
#     print("Ctrl-C -> Exit")
# finally:
#     env.close()
#     print("Done")
