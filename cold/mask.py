#!/usr/bin/env python3

import numpy as np
from scipy import signal
import logging


__author__ = "Doga Gursoy"
__copyright__ = "Copyright (c) 2021, UChicago Argonne, LLC."
__docformat__ = 'restructuredtext en'



def mask(mask):
    """Returns a mask."""
    grid = creategrid(mask)
    vals = gridvals(mask, grid)
    vals, grid = padmask(mask, vals, mask['pad'])
    return vals


def padmask(mask, vals, pad):
    vals = np.pad(vals, pad)
    padlen = 2 * pad * mask['resolution']
    totlen = masklength(mask) + padlen
    grid = np.arange(0, totlen, mask['resolution'])
    return vals, grid


def creategrid(mask):
    length = masklength(mask)
    grid = np.arange(0, length, mask['resolution'])
    logging.info("Mask grid created.")
    return grid


def masklength(mask):
    sequence = loadmask(mask)
    n1 = numones(sequence)
    n0 = numzeros(sequence)
    length =  np.dot((n0, n1), mask['bitsizes'])
    return length


def plotmask(mask, grid):
    """Plots the mask on a given grid."""
    import matplotlib.pyplot as plt
    plt.figure(figsize=(16, 1.5))
    plt.xlabel("Length [mu]")
    plt.ylabel("Mask")
    plt.plot(grid, mask)
    plt.grid(True)
    plt.tight_layout()
    plt.show()
    plt.close()


def loadmask(mask):
    return np.load(mask['path'])


def gridvals(mask, grid):
    sequence = np.load(mask['path'])
    vals = np.zeros(grid.shape, dtype='float32')
    nbits = np.size(sequence)
    pointer = 0 
    for m in range(nbits):
        bit = sequence[m]
        size = mask['bitsizes'][sequence[m]]
        try: 
            start = grid[pointer]
            while (grid[pointer] - start < size):
                vals[pointer] = bit
                pointer += 1
        except IndexError:
            pass
    logging.info("Grid values assigned.")
    if mask['widening'] > 0:
        vals = widen(mask, vals)
    if mask['smoothness'] > 0:
        vals = smooth(mask, vals)
    return vals


def widen(mask, vals):
    size = int(mask['widening'] / mask['resolution'])
    kernel = signal.tukey(size, alpha=0.0)
    kernel /= kernel.sum()
    vals = signal.convolve(vals, kernel, 'same')
    vals[vals > 0] = 1
    logging.info("Mask widened.")
    return vals


def smooth(mask, vals):
    size = int(mask['smoothness'] / mask['resolution'])
    kernel = signal.tukey(size, alpha=1.0)
    kernel /= kernel.sum()
    vals = signal.convolve(vals, kernel, 'same')
    logging.info("Mask smoothed.")
    return vals


def numbits(sequence):
    return np.size(sequence)


def numones(sequence):
    return np.sum(sequence)


def numzeros(sequence):
    return numbits(sequence) - numones(sequence)

def diffvals(mask):
    return -np.diff(mask)
