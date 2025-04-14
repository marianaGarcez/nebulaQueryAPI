#!/usr/bin/env python3
"""
Visualize NebulaStream query benchmark results across platforms.
This script generates bar charts comparing the performance metrics.
"""
import os
import sys
import glob
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def load_results_files(results_dir):
    """Load all benchmark summary CSV files from the results directory."""
    all_results = []
    files = glob.glob(os.path.join(results_dir, "benchmark_summary_*.csv"))
    
    if not files:
        print(f"No benchmark results found in {results_dir}")
        return None
    
    for file in files:
        try:
            # Extract platform name from filename
            platform = os.path.basename(file).split('_')[-1].split('.')[0]
            df = pd.read_csv(file)
            
            # If platform column doesn't exist, add it
            if 'Platform' not in df.columns:
                df['Platform'] = platform
                
            all_results.append(df)
        except Exception as e:
            print(f"Error reading {file}: {e}")
    
    if not all_results:
        return None
        
    # Combine all results
    combined_df = pd.concat(all_results, ignore_index=True)
    
    # Convert numeric columns to floats
    for col in ['Execution Time (s)', 'Max Memory (MB)', 'Avg Memory (MB)', 'Throughput (msgs/s)']:
        if col in combined_df.columns:
            combined_df[col] = pd.to_numeric(combined_df[col], errors='coerce')
    
    return combined_df

def plot_comparisons(df, output_dir, metric_columns=None):
    """Generate comparison bar charts for each metric."""
    if df is None or df.empty:
        print("No valid data to plot")
        return
    
    if metric_columns is None:
        metric_columns = ['Execution Time (s)', 'Max Memory (MB)', 'Avg Memory (MB)', 'Throughput (msgs/s)']
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Plotting colors
    colors = plt.cm.tab10.colors
    
    # For each metric, create a bar chart comparing platforms across queries
    for metric in metric_columns:
        if metric not in df.columns:
            continue
            
        plt.figure(figsize=(12, 8))
        
        # Convert to pivoted format for plotting
        pivot_df = df.pivot_table(
            index='Query', 
            columns='Platform', 
            values=metric,
            aggfunc='mean'
        )
        
        # Plot grouped bar chart
        ax = pivot_df.plot(kind='bar', color=colors, rot=0)
        
        plt.title(f'Comparison of {metric} Across Platforms')
        plt.ylabel(metric)
        plt.xlabel('Query')
        plt.grid(axis='y', linestyle='--', alpha=0.7)
        plt.legend(title='Platform')
        
        # Add value labels on the bars
        for container in ax.containers:
            ax.bar_label(container, fmt='%.2f', padding=3)
            
        # Adjust layout and save figure
        plt.tight_layout()
        plt.savefig(os.path.join(output_dir, f'comparison_{metric.split()[0].lower()}.png'))
        plt.close()
    
    # Create a summary table image
    plt.figure(figsize=(14, len(df) * 0.5 + 2))
    plt.axis('off')
    
    # Aggregate summary by platform and query
    summary = df.groupby(['Platform', 'Query']).mean().reset_index()
    
    # Create a formatted table
    cell_text = []
    for _, row in summary.iterrows():
        cell_text.append([
            row['Platform'], 
            row['Query'], 
            f"{row['Execution Time (s)']:.2f}", 
            f"{row['Max Memory (MB)']:.2f}", 
            f"{row['Avg Memory (MB)']:.2f}", 
            f"{row['Throughput (msgs/s)']:.2f}" if not np.isnan(row['Throughput (msgs/s)']) else "N/A"
        ])
    
    # Create table
    table = plt.table(
        cellText=cell_text,
        colLabels=['Platform', 'Query', 'Exec Time (s)', 'Max Mem (MB)', 'Avg Mem (MB)', 'Throughput (msgs/s)'],
        loc='center',
        cellLoc='center',
        colWidths=[0.15, 0.15, 0.15, 0.15, 0.15, 0.2]
    )
    
    # Style the table
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.5)
    
    plt.title('NebulaStream Query Benchmark Summary', fontsize=16, pad=20)
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'summary_table.png'), bbox_inches='tight')
    plt.close()
    
    print(f"Generated comparison charts in {output_dir}")

def main():
    # Get results directory from command line or use default
    if len(sys.argv) > 1:
        results_dir = sys.argv[1]
    else:
        # Try to find parent directory of the script to locate benchmark_results
        script_dir = os.path.dirname(os.path.abspath(__file__))
        default_results_dir = os.path.join(os.path.dirname(script_dir), "benchmark_results")
        
        if os.path.exists(default_results_dir):
            results_dir = default_results_dir
        else:
            results_dir = os.path.join(script_dir, "benchmark_results")
    
    print(f"Looking for benchmark results in: {results_dir}")
    
    # Set output directory for charts
    charts_dir = os.path.join(results_dir, "charts")
    
    # Load and plot results
    results_df = load_results_files(results_dir)
    if results_df is not None:
        plot_comparisons(results_df, charts_dir)
        print(f"Summary of results:\n{results_df.groupby(['Platform', 'Query']).mean().reset_index().to_string()}")
    else:
        print("No results to visualize.")

if __name__ == "__main__":
    main() 