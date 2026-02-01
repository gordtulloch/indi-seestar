#!/bin/bash

# Helper script to run test_bot with environment variables

if [ -z "$TOKEN" ] || [ -z "$SECRET" ]; then
    echo "Error: TOKEN and SECRET environment variables must be set"
    echo ""
    echo "Usage:"
    echo "  export TOKEN=your_token_here"
    echo "  export SECRET=your_secret_here"
    echo "  ./run_bot.sh <on|off>"
    echo ""
    echo "Or:"
    echo "  TOKEN=your_token SECRET=your_secret ./run_bot.sh <on|off>"
    exit 1
fi

if [ $# -ne 1 ]; then
    echo "Usage: $0 <on|off>"
    exit 1
fi

./build/test_bot "$1"
