#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# import gym
import gymnasium as gym
import argparse
import ns3gym
from ns3gym import ns3env
from stable_baselines3 import DQN

__author__ = "Jianpeng Qi"
__copyright__ = "Copyright (c) 2024, Ocean University of China"
__version__ = "0.1.0"
__email__ = "qijianpeng@ouc.edu.cn"


env = gym.make('ns3-v0')
# env = ns3env.Ns3Env()
ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

model = DQN("MlpPolicy", env, verbose=1)

model.learn(total_timesteps=1000, log_interval=4)

# model.save("dqn_opengym")

obs, info = env.reset(seed=99)
try:

    while True:
        action, _states = model.predict(obs, deterministic=True)
        obs, rewards, terminated, truncated, info = env.step(action)
        if terminated:
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
