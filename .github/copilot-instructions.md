<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

This project is a Node.js backend for water quality detection. It receives sensor data (turbidity, pH, ultrasonic distance), stores it in MongoDB, and provides a management view for data trends. If ultrasonic < 200, water is marked as 'insufficient', else 'sufficient'. Use EJS for rendering management views.
