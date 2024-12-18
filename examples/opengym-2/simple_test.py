#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import gymnasium as gym
import argparse
import ns3gym
from stable_baselines3 import A2C
__author__ = "Piotr Gawlowicz"
__copyright__ = "Copyright (c) 2018, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "gawlowicz@tkn.tu-berlin.de"

# env = gym.make("CartPole-v1", render_mode="rgb_array")
#
# model = A2C("MlpPolicy", env, verbose=1)
# model.learn(total_timesteps=10_000)
#
# vec_env = model.get_env()
# obs = vec_env.reset()
# for i in range(1000):
#     action, _state = model.predict(obs, deterministic=True)
#     obs, reward, done, info = vec_env.step(action)

env = gym.make('ns3-v0')
env.reset(seed=99)

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0

try:
    obs = env.reset()
    print("Step: ", stepIdx)
    print("---obs: ", obs)

    while True:
        stepIdx += 1

        action = env.action_space.sample()
        print("---action: ", action)
        obs, reward, terminated, truncated, info = env.step(action)

        print("Step: ", stepIdx)
        print("---obs, reward, terminated, info: ", obs, reward, terminated, info)
        myVector = obs["myVector"]
        myValue = obs["myValue"]
        print("---myVector: ", myVector)
        print("---myValue: ", myValue)

        if terminated:
            break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")
