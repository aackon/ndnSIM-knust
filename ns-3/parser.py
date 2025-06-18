#!/usr/bin/env python3
"""
XML Network Transmission Parser

This script parses NetAnim XML files and counts the number of transmissions from each node.
It identifies transmission events from <p> elements with fId (from ID) and tId (to ID) attributes.
"""

import xml.etree.ElementTree as ET
from collections import defaultdict
import argparse
import sys

def parse_xml_transmissions(file_path):
    """
    Parse XML file and count transmissions from each node.
    
    Args:
        file_path (str): Path to the XML file
        
    Returns:
        dict: Dictionary with node IDs as keys and transmission counts as values
    """
    try:
        # Parse the XML file
        tree = ET.parse(file_path)
        root = tree.getroot()
        
        # Dictionary to store transmission counts for each node
        transmission_counts = defaultdict(int)
        
        # Find all packet elements (p tags)
        packets = root.findall('.//p')
        
        print(f"Found {len(packets)} packet transmission events")
        
        # Count transmissions from each node
        for packet in packets:
            from_id = packet.get('fId')
            to_id = packet.get('tId')
            
            if from_id is not None:
                transmission_counts[int(from_id)] += 1
        
        return dict(transmission_counts)
        
    except ET.ParseError as e:
        print(f"Error parsing XML file: {e}")
        return {}
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return {}
    except Exception as e:
        print(f"Unexpected error: {e}")
        return {}

def get_node_info(file_path):
    """
    Extract basic node information from the XML file.
    
    Args:
        file_path (str): Path to the XML file
        
    Returns:
        dict: Dictionary with node information
    """
    try:
        tree = ET.parse(file_path)
        root = tree.getroot()
        
        nodes = {}
        node_elements = root.findall('.//node')
        
        for node in node_elements:
            node_id = int(node.get('id'))
            nodes[node_id] = {
                'id': node_id,
                'locX': node.get('locX'),
                'locY': node.get('locY'),
                'sysId': node.get('sysId')
            }
        
        return nodes
        
    except Exception as e:
        print(f"Error extracting node info: {e}")
        return {}

def print_results(transmission_counts, node_info):
    """
    Print the transmission results in a formatted way.
    
    Args:
        transmission_counts (dict): Dictionary with transmission counts
        node_info (dict): Dictionary with node information
    """
    print("\n" + "="*50)
    print("TRANSMISSION SUMMARY")
    print("="*50)
    
    if not transmission_counts:
        print("No transmissions found in the XML file.")
        return
    
    # Sort nodes by ID for consistent output
    sorted_nodes = sorted(transmission_counts.keys())
    
    total_transmissions = sum(transmission_counts.values())
    
    print(f"{'Node ID':<8} {'Location':<15} {'Transmissions':<12} {'Percentage':<10}")
    print("-" * 50)
    
    for node_id in sorted_nodes:
        count = transmission_counts[node_id]
        percentage = (count / total_transmissions) * 100 if total_transmissions > 0 else 0
        
        # Get location info if available
        location = "Unknown"
        if node_id in node_info:
            x = node_info[node_id]['locX']
            y = node_info[node_id]['locY']
            location = f"({x},{y})"
        
        print(f"{node_id:<8} {location:<15} {count:<12} {percentage:<10.1f}%")
    
    print("-" * 50)
    print(f"{'TOTAL':<24} {total_transmissions:<12} {'100.0%':<10}")
    
    # Additional statistics
    print(f"\nSTATISTICS:")
    print(f"Total nodes with transmissions: {len(transmission_counts)}")
    print(f"Average transmissions per node: {total_transmissions / len(transmission_counts):.1f}")
    print(f"Most active node: Node {max(transmission_counts, key=transmission_counts.get)} ({max(transmission_counts.values())} transmissions)")
    print(f"Least active node: Node {min(transmission_counts, key=transmission_counts.get)} ({min(transmission_counts.values())} transmissions)")

def main():
    """Main function to handle command line arguments and execute the parser."""
    parser = argparse.ArgumentParser(description='Parse NetAnim XML file and count transmissions per node')
    parser.add_argument('xml_file', help='Path to the XML file to parse')
    parser.add_argument('--export', '-e', help='Export results to CSV file')
    parser.add_argument('--verbose', '-v', action='store_true', help='Enable verbose output')
    
    args = parser.parse_args()
    
    if args.verbose:
        print(f"Parsing XML file: {args.xml_file}")
    
    # Parse transmissions and get node info
    transmission_counts = parse_xml_transmissions(args.xml_file)
    node_info = get_node_info(args.xml_file)
    
    # Print results
    print_results(transmission_counts, node_info)
    
    # Export to CSV if requested
    if args.export:
        export_to_csv(transmission_counts, node_info, args.export)
        print(f"\nResults exported to: {args.export}")

def export_to_csv(transmission_counts, node_info, filename):
    """
    Export results to CSV file.
    
    Args:
        transmission_counts (dict): Dictionary with transmission counts
        node_info (dict): Dictionary with node information
        filename (str): Output CSV filename
    """
    import csv
    
    try:
        with open(filename, 'w', newline='') as csvfile:
            fieldnames = ['node_id', 'location_x', 'location_y', 'transmissions']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            
            writer.writeheader()
            for node_id in sorted(transmission_counts.keys()):
                row = {
                    'node_id': node_id,
                    'location_x': node_info.get(node_id, {}).get('locX', 'Unknown'),
                    'location_y': node_info.get(node_id, {}).get('locY', 'Unknown'),
                    'transmissions': transmission_counts[node_id]
                }
                writer.writerow(row)
                
    except Exception as e:
        print(f"Error exporting to CSV: {e}")

if __name__ == "__main__":
    main()
